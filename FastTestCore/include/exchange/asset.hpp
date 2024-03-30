#pragma once
#include "ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct Asset {
  Vector<Int64> timestamps;
  Vector<double> data;
  Option<String> source;
  String name;
  size_t id;
  size_t rows = 0;
  size_t cols = 0;
  Map<String, size_t> headers;

  Asset(String _name, size_t _id) noexcept : name(std::move(_name)), id(_id) {}

  void resize(size_t _rows, size_t _cols) noexcept {
    timestamps.resize(_rows);
    data.resize(_rows * _cols);
    rows = _rows;
    cols = _cols;
  }

  void setSource(String _source) noexcept { source = std::move(_source); }

  bool isAscending() noexcept {
    for (size_t i = 1; i < timestamps.size(); ++i) {
      if (timestamps[i] < timestamps[i - 1]) {
        return false;
      }
    }
    return true;
  }
  FastTestResult<bool> loadCSV(String const &datetime_format);
};

END_FASTTEST_NAMESPACE