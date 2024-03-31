#include "py_wrap.hpp"
#include "py_exchange.hpp"
#include "py_manager.hpp"
#include "py_wrap_ast.hpp"

void wrap(py::module &m) noexcept {
  auto m_core = m.def_submodule("core");
  auto m_ast = m.def_submodule("ast");
  wrap_exchange(m_core);
  wrap_manager(m_core);

  wrap_ast_enum(m_ast);
  wrap_ast_buffer_node(m_ast);
  wrap_ast_observers(m_ast);
  wrap_ast_asset_node(m_ast);
  wrap_node_factory(m_ast);
}

PYBIND11_MODULE(fasttest_internal, m) { wrap(m); }
