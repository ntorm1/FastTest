#include "exchange/exchange.hpp"

#include "ast/ft_allocation.hpp"
#include "ast/ft_node_factory.hpp"
#include "strategy/ft_ast_strategy.hpp"
#include "strategy/ft_meta_strategy.hpp"
#include "strategy/ft_tracer.hpp"
#include "strategy/ft_measure.hpp"

#include "py_wrap.hpp"

using namespace FastTest;

void wrap_strategy(py::module &m_strategy) noexcept {
  py::class_<StrategyAllocatorConfig>(m_strategy, "StrategyAllocatorConfig")
      .def(py::init<>())
      .def_readwrite("can_short", &StrategyAllocatorConfig::can_short)
      .def_readwrite("allocation", &StrategyAllocatorConfig::allocation)
      .def_readwrite("disable_on_breach",
                     &StrategyAllocatorConfig::disable_on_breach)
      .def_readwrite("vol_target", &StrategyAllocatorConfig::vol_target)
      .def_readwrite("weight_clip", &StrategyAllocatorConfig::weight_clip)
      .def_readwrite("vol_limit", &StrategyAllocatorConfig::vol_limit)
      .def_readwrite("risk_contrib_limit",
                     &StrategyAllocatorConfig::risk_contrib_limit);

  py::class_<StrategyAllocator, std::shared_ptr<StrategyAllocator>>(
      m_strategy, "StrategyAllocator")
      .def("getTracer", &StrategyAllocator::getTracer)
      .def("getAssetAllocation", &StrategyAllocator::getAssetAllocation)
      .def("getAllocationBuffer", &StrategyAllocator::getAllocationBuffer,
           py::return_value_policy::reference);

  py::class_<MetaStrategy, StrategyAllocator, std::shared_ptr<MetaStrategy>>(
      m_strategy, "MetaStrategy")
      .def(py::init<const std::string &, const std::shared_ptr<Exchange> &,
                    const StrategyAllocatorConfig &, double,
                    const Option<std::shared_ptr<StrategyAllocator>> &>(),
           py::arg("name"), py::arg("exchange"), py::arg("config"),
           py::arg("starting_cash"),
           py::arg("parent") = std::nullopt)
      .def("addStrategy", &MetaStrategy::addStrategy, py::arg("strategy"),
           py::arg("replace_if_exsists"));

  py::class_<ASTStrategy, StrategyAllocator, std::shared_ptr<ASTStrategy>>(
      m_strategy, "ASTStrategy")
      .def(py::init<std::string, std::shared_ptr<StrategyAllocator>,
                    StrategyAllocatorConfig>(),
           py::arg("name"), py::arg("parent"), py::arg("config"))
      .def("getNodeFactory", &ASTStrategy::getNodeFactory);
}
