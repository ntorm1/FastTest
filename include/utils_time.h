#pragma once
#ifndef UTILS_TIME_H // include guard
#define UTILS_TIME_H

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <sys/time.h>
#endif 
#include <string>

//frequency definitions 
constexpr unsigned int SECOND1 = 1;
constexpr unsigned int SECOND5 = 5;
constexpr unsigned int SECOND30 = 30;
constexpr unsigned int MINUTE1 = 60;
constexpr unsigned int MINUTE5 = 300;
constexpr unsigned int MINUTE15 = 900;
constexpr unsigned int MINUTE30 = 1800;
constexpr unsigned int HOUR1 = 3600;
constexpr unsigned int HOUR2 = 7200;
constexpr unsigned int HOUR4 = 14400;
constexpr unsigned int DAY1 = 86400;
constexpr unsigned int US_EQUITY_DAY1 = 23400;

//open and close time definitions
constexpr unsigned int EST_US_EQUITY_HR_OPEN = 9;
constexpr unsigned int EST_US_EQUITY_MIN_OPEN = 30;
constexpr unsigned int EST_US_EQUITY_HR_CLOSE = 16;
constexpr unsigned int EST_US_EQUITY_MIN_CLOSE = 0;

constexpr unsigned int UTC_US_EQUITY_HR_OPEN = 14;
constexpr unsigned int UTC_US_EQUITY_MIN_OPEN = 30;
constexpr unsigned int UTC_US_EQUITY_HR_CLOSE = 17;
constexpr unsigned int UTC_US_EQUITY_MIN_CLOSE = 0;

#define MAX_TIME_LONG 2147483647
constexpr timeval MAX_TIME = { MAX_TIME_LONG,0 };

//function to parse a string to a timeval
void string_to_timeval(timeval *tv, std::string input_date, const char *digit_datetime_format, bool datetime = false);

//function to parse timeval to double 
double timeval_to_double(const timeval *tv);

//function for parsing a timeval to a string
size_t timeval_to_char_array(timeval *tv, char *buf, size_t sz);

//functions for comparing timevals 
bool operator > (const timeval &tv1, const timeval &tv2);
bool operator < (const timeval &tv1, const timeval &tv2);
bool operator == (const timeval &tv1, const timeval &tv2);
long operator - (const timeval &tv1, const timeval &tv2);
timeval operator + (const timeval &tv1, unsigned int seconds);
timeval operator + (const timeval &tv1, double seconds);
#endif