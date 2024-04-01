#pragma warning(disable : 4267)
#include "py_wrap.hpp"
#include "exchange/exchange.hpp"

using namespace FastTest;


void wrap_exchange(py::module &m_core) noexcept {
  py::class_<Exchange, std::shared_ptr<Exchange>>(m_core, "Exchange")
      .def("getCurrentIdx", &Exchange::getCurrentIdx)
      .def("getTimestamps", &Exchange::getTimestamps)
      .def("getAssetIndex", &Exchange::getAssetIndex)
      .def("getColumns", &Exchange::getColumns)
      .def("getData", &Exchange::getData)
      .def("getAssetMap", &Exchange::getAssetMap);
      //.def("getObserver", &Exchange::getObserver);

}

