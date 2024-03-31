
#include "py_exchange.hpp"
#include "exchange/exchange.hpp"

using namespace FastTest;

void wrap_exchange(py::module &m_core) noexcept {
  py::class_<Exchange, std::shared_ptr<Exchange>>(m_core, "Exchange")
      .def("getCurrentIdx", &Exchange::getCurrentIdx)
      .def("getTimestamps", &Exchange::getTimestamps)
      .def("getAssetIndex", &Exchange::getAssetIndex);
      //.def("getAssetMap", &Exchange::getAssetMap);
      //.def("getObserver", &Exchange::getObserver);
}

