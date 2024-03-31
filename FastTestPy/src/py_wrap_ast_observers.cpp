#include "ast/ft_ast_enums.hpp"
#include "ast/ft_observer.hpp"
#include "ast/ft_observer_base.hpp"
#include "py_manager.hpp"

using namespace FastTest::AST;

void wrap_ast_observers(py::module &m_ast) {

  py::class_<ObserverNode, BufferOpNode, std::shared_ptr<ObserverNode>>(
      m_ast, "ObserverNode");

  py::class_<SumObserverNode, ObserverNode, std::shared_ptr<SumObserverNode>>(
      m_ast, "SumObserverNode");
}