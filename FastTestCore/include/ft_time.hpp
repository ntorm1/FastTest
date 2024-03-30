#pragma once
#include "ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

namespace Time {

//============================================================================
enum class TimeUnit {
  DAYS = 0,
  WEEKS = 1,
  MONTHS = 2,
};

//============================================================================
struct TimeOffset {
  TimeUnit type;
  size_t count;
};

Int64 applyTimeOffset(Int64 t, TimeOffset o);
int getMonthFromEpoch(Int64 epoch) noexcept;
FastTestResult<Int64> strToEpoch(const String &str,
                                 const String &dt_format) noexcept;

//============================================================================
String convertNanosecondsToTime(Int64 nanoseconds) noexcept;

} // namespace Time

END_FASTTEST_NAMESPACE