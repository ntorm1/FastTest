#pragma once
#include "pch.hpp"

namespace py = pybind11;

void wrap(py::module &m) noexcept;
void wrap_ast_enum(py::module &m_ast);
void wrap_exchange(py::module &m) noexcept;
void wrap_strategy(py::module &m_strategy) noexcept;
void wrap_manager(py::module &m_core) noexcept;
