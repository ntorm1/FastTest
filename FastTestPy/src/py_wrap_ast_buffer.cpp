#include "ast/ft_ast_enums.hpp"
#include "ast/ft_buffer_node.hpp"
#include "ast/ft_allocation.hpp"
#include "py_wrap_ast.hpp"

using namespace FastTest::AST;

void wrap_ast_buffer_node(py::module &m_ast) {
  py::class_<BufferOpNode, std::shared_ptr<BufferOpNode>>(m_ast, "BufferOpNode")
      .def("enableCache", &BufferOpNode::enableCache)
      .def("hasCache", &BufferOpNode::hasCache)
      .def("getCache", &BufferOpNode::getCache,
           py::return_value_policy::reference_internal)
      .def("address", &BufferOpNode::address);

  py::class_<AllocationNode, BufferOpNode, std::shared_ptr<AllocationNode>>(
      m_ast, "AllocationNode");
}