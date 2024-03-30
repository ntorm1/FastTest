#include "ft_macros.hpp"
#include "exchange/exchange.hpp"
#include "exchange/exchange_map.hpp"
#include "ft_array.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct ExchangeMapImpl {
  Map<String, size_t> exchange_id_map;
  Map<size_t, String> asset_map;
  Vector<SharedPtr<Exchange>> exchanges;
  Vector<Int64> timestamps;
  Int64 global_time;
  size_t current_index = 0;
};

//============================================================================
void ExchangeMap::build() noexcept {
  m_impl->timestamps.clear();
  for (auto const &exchange : m_impl->exchanges) {
    m_impl->timestamps =
        sortedUnion(m_impl->timestamps, exchange->getTimestamps());
  }
}

//============================================================================
void ExchangeMap::reset() noexcept {
  m_impl->current_index = 0;
  m_impl->global_time = 0;
  for (auto &exchange : m_impl->exchanges) {
    exchange->reset();
  }
}

//============================================================================
void ExchangeMap::step() noexcept {}

//============================================================================
void ExchangeMap::cleanup() noexcept {}

//============================================================================
FastTestResult<SharedPtr<Exchange>>
ExchangeMap::addExchange(String name, String source,
                         Option<String> datetime_format) noexcept {
  FT_EXPECT_FALSE(m_impl->exchange_id_map.contains(name),
               "Exchange with name already exists");
  auto exchange =
      std::make_unique<Exchange>(std::move(name), std::move(source),
                                 m_impl->exchanges.size(), datetime_format);
  FT_EXPECT_TRUE(res, exchange->init());
  FT_EXPECT_TRUE(res_val, exchange->validate());
  FT_EXPECT_TRUE(res_build, exchange->build());
  m_impl->exchange_id_map[exchange->getName()] = m_impl->exchanges.size();
  m_impl->exchanges.push_back(std::move(exchange));

  // copy over exchange's asset map and set the exchange's index offset equal
  // to the asset map size
  auto& exchange_ptr = m_impl->exchanges.back();
  exchange_ptr->setExchangeOffset(m_impl->asset_map.size());
  for (auto const &asset : exchange_ptr->getAssetMap()) {
    m_impl->asset_map[asset.second] = asset.first;
  }
  return exchange_ptr;
}

//============================================================================
FastTestResult<SharedPtr<Exchange>>
ExchangeMap::getExchange(String const &name) const noexcept {
  if (!m_impl->exchange_id_map.contains(name)) {
		return Err("Exchange with name does not exist: ", name);
	}
	return m_impl->exchanges[m_impl->exchange_id_map.at(name)];
}

//============================================================================
ExchangeMap::ExchangeMap() noexcept {
  m_impl = std::make_unique<ExchangeMapImpl>();
}

//============================================================================
ExchangeMap::~ExchangeMap() noexcept {}

END_FASTTEST_NAMESPACE
