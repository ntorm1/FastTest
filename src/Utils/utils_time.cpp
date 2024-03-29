#include <iostream>
#ifdef _WIN32
#include "pch.h"
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif 
#include <stdio.h>
#include <ctime>
#include <cmath>
#include <string>
#include <assert.h>
#include "utils_time.h"
 
const char *_datetime_format = "%Y-%m-%d %H:%M:%S";

bool operator > (const timeval &tv1, const timeval &tv2) {
	if (tv1.tv_sec > tv2.tv_sec) { return true; }
	if (tv1.tv_sec < tv2.tv_sec) { return false; }
	if (tv1.tv_usec > tv2.tv_usec) { return true; }
	return false;
}
bool operator < (const timeval &tv1, const timeval &tv2) {
	if (tv1.tv_sec < tv2.tv_sec) { return true; }
	if (tv1.tv_sec > tv2.tv_sec) { return false; }
	if (tv1.tv_usec < tv2.tv_usec) { return true; }
	return false;
}
bool operator == (const timeval &tv1, const timeval &tv2) {
	if (tv1.tv_sec != tv2.tv_sec) { return false; }
	if (tv1.tv_sec != tv2.tv_sec) { return false; }
	return true;
}
long operator - (const timeval &tv1, const timeval &tv2) {
	long seconds = tv1.tv_sec + tv2.tv_sec;
	seconds += ((tv1.tv_usec + tv2.tv_usec)/1e6);
	return seconds;
}
timeval operator + (const timeval &tv1, unsigned int seconds) {
	seconds = tv1.tv_sec + seconds;
	return timeval{seconds,tv1.tv_usec};
}

timeval operator + (const timeval &tv1, double seconds) {
	double whole, fractional;
	long tv_sec;
	int tv_usec;
	
	fractional = std::modf(seconds, &whole);
	tv_sec = static_cast<long>(whole);
	fractional *= 1e6;
	tv_usec = static_cast<long>(fractional);
	
	return timeval{tv1.tv_sec+tv_sec,tv1.tv_usec+tv_usec};
}


/*
void gettimeofday(timeval *tp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970
	SYSTEMTIME  system_time;
	FILETIME    file_time;
	unsigned int64_t    time;
	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((unsigned int64_t)file_time.dwLowDateTime);
	time += ((unsigned int64_t)file_time.dwHighDateTime) << 32;
	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
}
*/
void string_to_timeval(timeval *tv, std::string input_date, const char *digit_datetime_format, bool datetime)
{
	int yy, month, dd;
	int hh = 0; int mm = hh; int ss = hh;
	struct tm datetime_tm;
	int decimal_location = input_date.find(".");

	if (decimal_location == -1) {
		if (!datetime) {
			sscanf(input_date.c_str(), digit_datetime_format, &yy, &month, &dd);
		}
		else {
			sscanf(input_date.c_str(), digit_datetime_format, &yy, &month, &dd, &hh, &mm, &ss);
		}
		tv->tv_usec = 0;
	}
	else {
		std::string datetime = input_date.substr(0, decimal_location);
		sscanf(datetime.c_str(), digit_datetime_format, &yy, &month, &dd, &hh, &mm, &ss);
		tv->tv_usec = std::stol(input_date.substr(decimal_location + 1)) * 10000;
	}
	datetime_tm.tm_year = yy - 1900;
	datetime_tm.tm_mon = month - 1;
	datetime_tm.tm_mday = dd;
	datetime_tm.tm_hour = hh;
	datetime_tm.tm_min = mm;
	datetime_tm.tm_sec = ss;
	datetime_tm.tm_isdst = -1;

	long tStart = (long)timegm(&datetime_tm);
	tv->tv_sec = tStart;
}

double timeval_to_double(const timeval *tv){
	double fractional = tv->tv_usec / 1e6;
	double return_time = static_cast<double>(tv->tv_sec + fractional);
	return return_time;
}

size_t timeval_to_char_array(timeval *tv, char *buf, size_t sz)
{
	size_t written = 0;
	const time_t time = *&tv->tv_sec;
	tm bt;
#ifdef __GNUC__
	localtime_r(&time, &bt);
#elif _MSC_VER
	localtime_s(&bt, &time);
#endif 
	if (time)
	{
		written = (size_t)strftime(buf, sz, _datetime_format, &bt);
		if ((written > 0) && ((size_t)written < sz))
		{
			int w = snprintf(buf + written, sz - (size_t)written, ".%06d", tv->tv_usec);
			written = (w > 0) ? written + w : -1;
		}
	}
	return written;
}
