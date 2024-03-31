#include "exchange/exchange.hpp"
#include "ast/ft_ast_enums.hpp"
#include "ast/ft_node_factory.hpp"
#include "ast/ft_asset_node.hpp"
#include "ast/ft_observer.hpp"
#include "py_manager.hpp"

using namespace FastTest::AST;

void wrap_node_factory(py::module &m_ast) {
  py::class_<NodeFactory, std::shared_ptr<NodeFactory>>(m_ast, "NodeFactory")
      .def(py::init<std::shared_ptr<FastTest::Exchange>>(), py::arg("exchange")) 
      .def("createReadOpNode", &NodeFactory::createReadOpNode,
           py::arg("column"), py::arg("row_offset"))
      .def("createBinOpNode", &NodeFactory::createBinOpNode, py::arg("left"),
           py::arg("op"), py::arg("right"))
      .def("createUnaryOpNode", &NodeFactory::createUnaryOpNode,
           py::arg("parent"),
           py::arg("op"), py::arg("op_param"))
      .def("createSumObserverNode", &NodeFactory::createSumObserverNode,
           py::arg("node"), py::arg("window"), py::arg("name") = py::none());
}