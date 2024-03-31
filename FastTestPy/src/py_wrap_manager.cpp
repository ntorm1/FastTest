#include "py_manager.hpp"
#include "manager/ft_manager.hpp"
#include "exchange/exchange.hpp"

using namespace FastTest;

void wrap_manager(py::module &m_core) noexcept {
  py::enum_<FTManagerState>(m_core, "FTManagerState")
      .value("INIT", FTManagerState::INIT)
      .value("BUILT", FTManagerState::BUILT)
      .value("RUNNING", FTManagerState::RUNING)
      .value("FINISHED", FTManagerState::FINISHED)
      .export_values();

  py::class_<FTManager, std::shared_ptr<FTManager>>(m_core, "FTManager")
      .def(py::init<>())
      .def("addExchange", &FTManager::addExchange, py::arg("name"),
           py::arg("source"), py::arg("datetime_format") = py::none())
      .def("getExchange", &FTManager::getExchange, py::arg("name"))
      .def("getGlobalTime", &FTManager::getGlobalTime)
      .def("step", &FTManager::step)
      .def("reset", &FTManager::reset);
}