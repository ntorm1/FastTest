#include "exchange/exchange.hpp"
#include "ast/ft_ast_enums.hpp"
#include "ast/ft_node_factory.hpp"
#include "ast/ft_asset_node.hpp"
#include "ast/ft_observer.hpp"
#include "ast/ft_allocation.hpp"
#include "strategy/ft_allocator.hpp"
#include "py_wrap_ast.hpp"

using namespace FastTest::AST;

void wrap_node_factory(py::module &m_ast) {
  py::class_<NodeFactory, std::shared_ptr<NodeFactory>>(m_ast, "NodeFactory")
      .def("createReadOpNode", &NodeFactory::createReadOpNode,
           py::arg("column"), py::arg("row_offset") = 0)
      .def("createBinOpNode", &NodeFactory::createBinOpNode, py::arg("left"),
           py::arg("op"), py::arg("right"))
      .def("createUnaryOpNode", &NodeFactory::createUnaryOpNode,
           py::arg("parent"),
           py::arg("op"), py::arg("op_param"))
      .def("createAllocationNode", &NodeFactory::createAllocationNode,
           py::arg("parent"), py::arg("alloc_type"),py::arg("epsilon") = 0.0f, py::arg("alloc_param") = py::none())
      .def("createSumObserverNode", &NodeFactory::createSumObserverNode,
           py::arg("node"), py::arg("window"), py::arg("name") = py::none());
}