#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "standard/ft_macros.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
enum class BinOpType { ADD, SUB, MUL, DIV };

//============================================================================
enum class UnaryOpType { SCALAR = 0, SIGN = 1, POWER = 2, ABS = 3, LOG = 4 };

//============================================================================
enum class ReduceOpType { GREATER_THAN = 0, LESS_THAN = 1, EQUAL = 2 };

//============================================================================
enum class AllocationType {
  UNIFORM = 0,
  CONDITIONAL_SPLIT = 1,
  FIXED = 2,
  NLARGEST = 3,
  NSMALLEST = 4,
  NEXTREME = 5,
  INPLACE = 6,
};


//============================================================================
enum class ObserverType {
  SUM = 0,
  MEAN = 1,
  ATR = 2,
  MAX = 3,
  TS_ARGMAX = 4,
  VARIANCE = 5,
  COVARIANCE = 6,
  CORRELATION = 7,
  LINEAR_DECAY = 8,
  SKEWNESS = 9,
};

END_AST_NAMESPACE
