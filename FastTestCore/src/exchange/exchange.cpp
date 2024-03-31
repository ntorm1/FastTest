#include <format>

#include "ast/ft_base_node.hpp"
#include "ast/ft_observer_base.hpp"
#include "exchange/exchange.hpp"
#include "exchange/exchange_impl.hpp"
#include "strategy/ft_allocator.hpp"

#include "ft_array.hpp"
#include "ft_time.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
Exchange::~Exchange() noexcept {}

//============================================================================
Exchange::Exchange(String name, String source, size_t id,
                   Option<String> datetime_format) noexcept
    : m_name(name), m_source(source), m_id(id) {
  m_impl = std::make_unique<ExchangeImpl>();
  m_impl->datetime_format = datetime_format;
}

//============================================================================
void Exchange::registerAllocator(
    NonNullPtr<StrategyAllocator> allocator) noexcept {
  for (auto const &alloc : m_impl->allocators) {
    if (alloc.get() == allocator.get()) {
      return;
    }
  }
  m_impl->allocators.push_back(allocator);
}

//============================================================================
static Option<size_t>
getCloseIndex(const Map<std::string, size_t> &headers) noexcept {
  Option<size_t> close_index = std::nullopt;
  for (auto const &[key, value] : headers) {
    auto toLower = [](const std::string &str) -> std::string {
      std::string lower_string;
      lower_string.resize(str.size());
      for (size_t i = 0; i < str.size(); i++) {
        lower_string[i] = std::tolower(str[i]);
      }
      return lower_string;
    };
    if (toLower(key) == "close") {
      close_index = value;
    }
  }
  return close_index;
}

//============================================================================
FastTestResult<bool> Exchange::validate() noexcept {
  for (auto const &asset : m_impl->assets) {
    // validate all assets have the same headers
    auto const &asset_headers = asset.headers;
    if (m_impl->headers.size() == 0) {
      m_impl->headers = asset_headers;
      m_impl->col_count = asset_headers.size();
    } else {
      for (auto const &[key, value] : asset_headers) {
        if (m_impl->headers.count(key) == 0) {
          return Err("Asset {} has different headers than exchange",
                     asset.name);
        }
      }
    }
    // validate asset timestamps are in ascending order
    auto const &timestamps = asset.timestamps;
    if (timestamps.size() == 0) {
      return Err("Asset {} has no timestamps", asset.name);
    }
    for (size_t i = 1; i < timestamps.size(); i++) {
      bool is_descending = timestamps[i] < timestamps[i - 1];
      if (is_descending) {
        String message = std::format(
            "Asset timestamps are not in ascending order: i-1: {}, i: {}",
            Time::convertNanosecondsToTime(timestamps[i - 1]),
            Time::convertNanosecondsToTime(timestamps[i]));
        return tl::unexpected(message);
      };
    }
    m_impl->asset_id_map[asset.name] = m_impl->asset_id_map.size();
    m_impl->timestamps = sortedUnion(m_impl->timestamps, timestamps);
  }
  for (auto const &asset : m_impl->assets) {
    // validate each asset's timestamps are a contiguous subset of the exchange
    // timestamps
    auto const &timestamps = asset.timestamps;
    if (!isContinuousSubset(m_impl->timestamps, timestamps)) {
      return Err("Asset {} timestamps are not a contiguous subset of exchange",
                 asset.name);
    }
  }
  Option<size_t> close_index = getCloseIndex(m_impl->headers);
  if (!close_index.has_value()) {
    return Err("Exchange does not have a close price header");
  }
  m_impl->close_index = close_index.value();
  return true;
}

//============================================================================
FastTestResult<bool> Exchange::build() noexcept {
  // eigen stores data in column major order, so the exchange's data
  // matrix has rows = #assets, cols = #timestamps * #headers
  m_impl->data.resize(m_impl->assets.size(),
                      m_impl->timestamps.size() * m_impl->headers.size());
  // store the percentage change in price for each asset at each timestamp
  m_impl->returns.resize(m_impl->assets.size(), m_impl->timestamps.size());
  // store 1 + the percentage change in price for each asset at the current
  // timestamp
  m_impl->returns_scaler.resize(m_impl->assets.size());
  m_impl->returns_scaler.setZero();

  for (auto const &asset : m_impl->assets) {
    size_t asset_index = 0;
    size_t asset_id = asset.id;
    auto const &asset_datetime_index = asset.timestamps;
    for (size_t exchange_index = 0; exchange_index < m_impl->timestamps.size();
         exchange_index++) {
      auto exchange_datetime = m_impl->timestamps[exchange_index];
      size_t asset_datetime = 0;
      if (asset_index < asset_datetime_index.size()) {
        asset_datetime = asset_datetime_index[asset_index];
      }
      // asset datetime is out of bounds or does not match exchange datetime
      if (!asset_datetime || exchange_datetime != asset_datetime) {
        // fill data matrix with NAN
        for (size_t i = 0; i < m_impl->headers.size(); i++) {
          m_impl->data(asset_id, exchange_index * m_impl->headers.size() + i) =
              NAN_DOUBLE;
        }

        // fill returns matrix with 0
        m_impl->returns(asset_id, exchange_index) = 0;
        // update null count
      } else {
        // copy asset data into exchange data matrix
        for (size_t i = 0; i < m_impl->headers.size(); i++) {
          size_t asset_data_index = asset_index * m_impl->headers.size() + i;
          double value = asset.data[asset_data_index];
          size_t data_index = exchange_index * m_impl->headers.size() + i;
          m_impl->data(asset_id, data_index) = value;
        }

        // calculate returns
        if (!asset_index) {
          m_impl->returns(asset_id, exchange_index) = 0.0;
        } else {
          double prev_close = m_impl->data(
              asset_id, (exchange_index - 1) * m_impl->headers.size() +
                            m_impl->close_index);
          double curr_close =
              m_impl->data(asset_id, exchange_index * m_impl->headers.size() +
                                         m_impl->close_index);
          double ret = (curr_close - prev_close) / prev_close;
          if (ret != ret) {
            ret = 0.0;
          }
          m_impl->returns(asset_id, exchange_index) = ret;
        }
        asset_index++;
      }
    }
  }
  m_impl->assets.clear();
  return true;
}

//============================================================================
void Exchange::setExchangeOffset(size_t _offset) noexcept {
  m_impl->setExchangeOffset(_offset);
}

//============================================================================
Map<String, size_t> const &Exchange::getAssetMap() const noexcept {
  return m_impl->asset_id_map;
}

//============================================================================
void Exchange::reset() noexcept {
  m_impl->current_index = 0;
  m_impl->current_timestamp = 0;
  for (auto &observer : m_impl->observers) {
    observer->resetBase();
  }
}

//============================================================================
void Exchange::step(Int64 global_time) noexcept {
  if (m_impl->current_index >= m_impl->timestamps.size()) {
    return;
  }
  m_impl->current_timestamp = m_impl->timestamps[m_impl->current_index];
  if (m_impl->current_timestamp != global_time) {
    return;
  }
  for (auto &allocator : m_impl->allocators) {
    allocator->setStepCall(true);
  }
  m_impl->current_index++;
#pragma omp parallel for
  for (auto &observer : m_impl->observers) {
    observer->cacheBase();
  }
}

//============================================================================
size_t Exchange::getCurrentIdx() {
  assert(m_impl->current_index > 0);
  return m_impl->current_index - 1;
}

//============================================================================
Vector<Int64> const &Exchange::getTimestamps() const noexcept {
  return m_impl->timestamps;
}

//============================================================================
Option<size_t> Exchange::getAssetIndex(String const &asset) const noexcept {
  if (m_impl->asset_id_map.count(asset) == 0) {
    return std::nullopt;
  }
  return m_impl->asset_id_map.at(asset);
}

//============================================================================
size_t Exchange::getAssetCount() const noexcept {
  return m_impl->asset_id_map.size();
}

//============================================================================
static void linkObserver(Vector<NonNullPtr<AST::ASTNode>> &parents,
                         SharedPtr<AST::ObserverNode> observer,
                         int &parent_count) noexcept {
  for (auto const &parent : parents) {
    if (parent->getType() == AST::NodeType::ASSET_OBSERVER) {
      auto asset_observer = static_cast<AST::ObserverNode *>(parent.get());
      asset_observer->insertChild(observer);
      parent_count++;
    }
    auto &grandparents = parent->getParentsMut();
    for (auto &grandparent : grandparents) {
      linkObserver(grandparent->getParentsMut(), observer, parent_count);
    }
  }
}

//============================================================================
SharedPtr<AST::ObserverNode>
Exchange::registerObserver(SharedPtr<AST::ObserverNode> &&node) noexcept {
  for (auto const &other : m_impl->observers) {
    if (other->isSame(node.get())) {
      return other;
    }
  }
  int parent_count = 0;
  linkObserver(node->getParentsMut(), node, parent_count);
  node->m_parent_observer_max = parent_count;
  m_impl->observers.push_back(node);
  return node;
}

//============================================================================
Option<size_t> Exchange::getColumnIndex(String const &column) const noexcept {
  if (m_impl->headers.count(column) == 0) {
    return std::nullopt;
  }
  return m_impl->headers.at(column);
}

//============================================================================
LinAlg::EigenMatrixXd const &Exchange::getData() const noexcept {
  return m_impl->data;
}

//============================================================================
LinAlg::EigenVectorXd const &Exchange::getReturnsScalar() const noexcept {
  return m_impl->returns_scaler;
}

//============================================================================
LinAlg::EigenConstColView<double>
Exchange::getSlice(size_t column, int row_offset) const noexcept {
  assert(m_impl->current_index > 0);
  size_t idx = ((m_impl->current_index - 1) * m_impl->col_count) + column;
  if (row_offset) {
    size_t offset = abs(row_offset) * m_impl->col_count;
    assert(idx >= offset);
    idx -= offset;
  }
  assert(idx < static_cast<size_t>(m_impl->data.cols()));
  return m_impl->data.col(idx);
}

//============================================================================
LinAlg::EigenBlockView<double>
Exchange::getMarketReturnsBlock(size_t start_idx,
                                size_t end_idx) const noexcept {
  assert(start_idx < end_idx);
  assert(end_idx < static_cast<size_t>(m_impl->returns.cols()));
  return m_impl->returns(Eigen::indexing::all, Eigen::seq(start_idx, end_idx));
}

//============================================================================
LinAlg::EigenConstColView<double>
Exchange::getMarketReturns(int offset) const noexcept {
  assert(m_impl->current_index > 0);
  assert(offset <= 0);
  assert(static_cast<size_t>(abs(offset)) <= m_impl->current_index - 1);
  assert(m_impl->current_index - 1 + offset <
         static_cast<size_t>(m_impl->returns.cols()));
  return m_impl->returns.col(m_impl->current_index - 1 + offset);
}

//============================================================================
LinAlg::EigenConstRowView<double>
Exchange::getAssetSlice(size_t asset_index) const noexcept {
  assert(asset_index < static_cast<size_t>(m_impl->data.rows()));
  return m_impl->data.row(asset_index);
}

END_FASTTEST_NAMESPACE
