#include "py_wrap.hpp"
#include "py_exchange.hpp"
#include "py_manager.hpp"

void wrap(py::module &m) noexcept {
  auto m_core = m.def_submodule("core");
  wrap_exchange(m_core);
  wrap_manager(m_core);
}

PYBIND11_MODULE(fasttest_internal, m) { wrap(m); }
