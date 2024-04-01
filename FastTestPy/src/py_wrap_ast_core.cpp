#include "py_wrap.hpp"
#include "ast/ft_ast_enums.hpp"

using namespace FastTest::AST;

void wrap_ast_enum(py::module &m_ast) {
  py::enum_<BinOpType>(m_ast, "BinOpType")
        .value("ADD", BinOpType::ADD)
        .value("SUB", BinOpType::SUB)
        .value("MUL", BinOpType::MUL)
        .value("DIV", BinOpType::DIV);

    py::enum_<UnaryOpType>(m_ast, "UnaryOpType")
        .value("SCALAR", UnaryOpType::SCALAR)
        .value("SIGN", UnaryOpType::SIGN)
        .value("POWER", UnaryOpType::POWER)
        .value("ABS", UnaryOpType::ABS)
        .value("LOG", UnaryOpType::LOG);

    py::enum_<ReduceOpType>(m_ast, "ReduceOpType")
        .value("GREATER_THAN", ReduceOpType::GREATER_THAN)
        .value("LESS_THAN", ReduceOpType::LESS_THAN)
        .value("EQUAL", ReduceOpType::EQUAL);

    py::enum_<NodeType>(m_ast, "NodeType")
        .value("BIN_OP", NodeType::BIN_OP)
        .value("UNARY_OP", NodeType::UNARY_OP)
        .value("ASSET_READ", NodeType::ASSET_READ)
        .value("ASSET_OBSERVER", NodeType::ASSET_OBSERVER)
        .value("REDUCE_OP", NodeType::REDUCE_OP)
        .value("ALLOCATION", NodeType::ALLOCATION);

    py::enum_<AllocationType>(m_ast, "AllocationType")
        .value("UNIFORM", AllocationType::UNIFORM)
        .value("CONDITIONAL_SPLIT", AllocationType::CONDITIONAL_SPLIT)
        .value("FIXED", AllocationType::FIXED)
        .value("NLARGEST", AllocationType::NLARGEST)
        .value("NSMALLEST", AllocationType::NSMALLEST)
        .value("NEXTREME", AllocationType::NEXTREME)
        .value("INPLACE", AllocationType::INPLACE);

    py::enum_<ObserverType>(m_ast, "ObserverType")
        .value("SUM", ObserverType::SUM)
        .value("MEAN", ObserverType::MEAN)
        .value("ATR", ObserverType::ATR)
        .value("MAX", ObserverType::MAX)
        .value("TS_ARGMAX", ObserverType::TS_ARGMAX)
        .value("VARIANCE", ObserverType::VARIANCE)
        .value("COVARIANCE", ObserverType::COVARIANCE)
        .value("CORRELATION", ObserverType::CORRELATION)
        .value("LINEAR_DECAY", ObserverType::LINEAR_DECAY)
        .value("SKEWNESS", ObserverType::SKEWNESS);
  }