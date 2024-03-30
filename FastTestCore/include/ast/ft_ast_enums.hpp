#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif

BEGIN_AST_NAMESPACE

//============================================================================
enum class BinOpType { ADD, SUB, MUL, DIV };

END_AST_NAMESPACE
