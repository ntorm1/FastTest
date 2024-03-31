#pragma once
#include "pch.hpp"

namespace py = pybind11;


void wrap_ast_enum(py::module &m_ast);
void wrap_node_factory(py::module &m_ast);
void wrap_ast_buffer_node(py::module &m_ast);
void wrap_ast_asset_node(py::module &m_ast);
void wrap_ast_observers(py::module &m_ast);
