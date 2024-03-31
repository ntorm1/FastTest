#include "ast/ft_asset_node.hpp"
#include "ast/ft_reduce.hpp"
#include "py_wrap_ast.hpp"

using namespace FastTest::AST;

void wrap_ast_asset_node(py::module &m_ast) {
  py::class_<ReadOpNode, BufferOpNode, std::shared_ptr<ReadOpNode>>(
      m_ast, "ReadOpNode");

  py::class_<BinOpNode, BufferOpNode, std::shared_ptr<BinOpNode>>(m_ast,
                                                                  "BinOpNode");

  py::class_<UnaryOpNode, BufferOpNode, std::shared_ptr<UnaryOpNode>>(
      m_ast, "UnaryOpNode");

  py::class_<ReduceOpNode, BufferOpNode, std::shared_ptr<ReduceOpNode>>(
      m_ast, "ReduceOpNode");



}