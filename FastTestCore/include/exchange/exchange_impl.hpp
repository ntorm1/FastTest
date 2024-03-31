#pragma once
#include "exchange/asset.hpp"
#include "standard/ft_linalg.hpp"
#include "standard/ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

class Exchange;

//============================================================================
struct ExchangeImpl {
  friend class Exchange;

private:
  void setExchangeOffset(size_t _offset) noexcept { exchange_offset = _offset; }

public:
  Map<String, size_t> asset_id_map;
  Map<String, size_t> headers;
  Vector<Asset> assets;
  Vector<Int64> timestamps;
  Vector<NonNullPtr<StrategyAllocator>> allocators;
  Vector<SharedPtr<AST::ObserverNode>> observers;
  Option<String> datetime_format = std::nullopt;
  LinAlg::EigenMatrixXd data;
  LinAlg::EigenMatrixXd mask;
  LinAlg::EigenVectorXi mask_required;
  LinAlg::EigenMatrixXd returns;
  LinAlg::EigenVectorXd returns_scaler;
  Int64 current_timestamp = 0;
  size_t exchange_offset = 0;
  size_t col_count = 0;
  size_t close_index = 0;
  size_t current_index = 0;

  ExchangeImpl() noexcept { data = LinAlg::EigenMatrixXd::Zero(0, 0); }
};

END_FASTTEST_NAMESPACE
