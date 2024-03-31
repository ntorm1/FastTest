#include "exchange/exchange.hpp"
#include "manager/ft_manager.hpp"
#include "py_manager.hpp"

using namespace FastTest;

void wrap_manager(py::module &m_core) noexcept {
  py::enum_<FTManagerState>(m_core, "FTManagerState")
      .value("INIT", FTManagerState::INIT)
      .value("BUILT", FTManagerState::BUILT)
      .value("RUNNING", FTManagerState::RUNING)
      .value("FINISHED", FTManagerState::FINISHED)
      .export_values();

  py::class_<FTManager, std::shared_ptr<FTManager>>(m_core, "FTManager")
      .def(py::init<>(), "Initialize the FTManager instance.")
      .def("addExchange", &FTManager::addExchange, py::arg("name"),
           py::arg("source"), py::arg("datetime_format") = py::none(),
           "Add an exchange to the FTManager instance.\n"
           "Args:\n"
           "    name (str): The name of the exchange.\n"
           "    source (str): The source of the exchange data.\n"
           "    datetime_format (str, optional): The datetime format of the "
           "exchange data. Defaults to None.")
      .def("getExchange", &FTManager::getExchange, py::arg("name"),
           "Get the exchange by its name.\n"
           "Args:\n"
           "    name (str): The name of the exchange to retrieve.")
      .def("getGlobalTime", &FTManager::getGlobalTime,
           "Get the global time from the FTManager instance.")
      .def("step", &FTManager::step,
           "Proceed to the next time step in the FTManager instance.")
      .def("reset", &FTManager::reset,
           "Reset the FTManager instance, reverting it to its initial state.");
}