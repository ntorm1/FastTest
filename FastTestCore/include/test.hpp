#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif

namespace FastTest {

[[nodiscard]] FASTTEST_API int addTwoNumbers(int a, int b);

}