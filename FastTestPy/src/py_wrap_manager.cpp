#include "exchange/exchange.hpp"
#include "manager/ft_manager.hpp"
#include "strategy/ft_meta_strategy.hpp"
#include "py_wrap.hpp"

using namespace FastTest;


static void wrap_exception(py::module &m_core) noexcept {
  py::class_<FastTestException>(m_core, "FastTestException")
      .def(py::init<std::string>())
      .def_property_readonly("message", &FastTestException::what);
}

void wrap_manager(py::module &m_core) noexcept {
  py::enum_<FTManagerState>(m_core, "FTManagerState")
      .value("INIT", FTManagerState::INIT)
      .value("BUILT", FTManagerState::BUILT)
      .value("RUNNING", FTManagerState::RUNING)
      .value("FINISHED", FTManagerState::FINISHED)
      .export_values();

  wrap_exception(m_core);

  py::class_<FTManager, std::shared_ptr<FTManager>>(m_core, "FTManager")
      .def(py::init<>(), "Initialize the FTManager instance.")
      .def("addExchange", &FTManager::addExchange, py::arg("name"),
           py::arg("source"), py::arg("datetime_format") = py::none(),
           "Add an exchange to the FTManager instance.\n\n"
           ":param str name: The name of the exchange.\n"
           ":param str source: The source of the exchange data.\n"
           ":param str datetime_format: The datetime format of the exchange "
           "data. Defaults to None.")
      .def("addStrategy", &FTManager::addStrategy, py::arg("strategy"),
           py::arg("replace_if_exsists"))
      .def("getExceptions", &FTManager::getExceptions)
      .def("getState", &FTManager::getState)
      .def("getExchange", &FTManager::getExchange, py::arg("name"),
           "Get the exchange by its name.\n\n"
           ":param str name: The name of the exchange to retrieve.")
      .def("build", &FTManager::build)
      .def("run", &FTManager::run)
      .def("getGlobalTime", &FTManager::getGlobalTime,
           "Get the global time from the FTManager instance.")
      .def("step", &FTManager::step,
           "Proceed to the next time step in the FTManager instance.")
      .def("reset", &FTManager::reset,
           "Reset the FTManager instance, reverting it to its initial state.");
}