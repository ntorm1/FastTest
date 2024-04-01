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


#define ADD_EXCEPTION_TO_IMPL(msg)                                             \
  m_impl->exceptions.push_back(FastTestException(msg));                        \


#define ADD_FORMATED_EXCEPTION(...)                                             \
  m_impl->exceptions.push_back(FastTestException(std::format(__VA_ARGS__)));

#define CHECK_EXCEPTIONS                                                       \
  if (m_impl->exceptions.size()) {                                             \
    return std::nullopt;                                                      \
  } 
