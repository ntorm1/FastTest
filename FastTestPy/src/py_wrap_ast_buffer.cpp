#include "ast/ft_ast_enums.hpp"
#include "ast/ft_buffer_node.hpp"
#include "py_manager.hpp"

using namespace FastTest::AST;

void wrap_ast_buffer_node(py::module &m_ast) {
  py::class_<BufferOpNode, std::shared_ptr<BufferOpNode>>(m_ast, "BufferOpNode")
      .def("enableCache", &BufferOpNode::enableCache)
      .def("hasCache", &BufferOpNode::hasCache)
      .def("getCache", &BufferOpNode::getCache,
           py::return_value_policy::reference_internal)
      .def("address", &BufferOpNode::address);
}