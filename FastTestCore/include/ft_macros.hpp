#pragma once

#define BEGIN_FASTTEST_NAMESPACE namespace FastTest {

#define END_FASTTEST_NAMESPACE } // namespace FastTest

#define BEGIN_AST_NAMESPACE                                                    \
  BEGIN_FASTTEST_NAMESPACE namespace AST {
#define END_AST_NAMESPACE }  \
 }// namespace AST

#define Err(...) tl::unexpected(std::format(__VA_ARGS__))

#define NAN_DOUBLE std::numeric_limits<double>::quiet_NaN()

#define FT_EXPECT_TRUE(val, expr)                                                 \
  auto val = expr;                                               \
  if (!val) {                                                    \
    return Err<FastTest::FastTestException>(val.error());              \
  }                                                                            \
  \

#define FT_EXPECT_FALSE(expr, msg)                                                \
  do {                                                                         \
    if (expr) {                                                                \
      return Err<FastTest::FastTestException>(msg);                                         \
    }                                                                          \
  } while (false)
