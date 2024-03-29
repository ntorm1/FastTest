#ifdef _WIN32
#include "pch.h"
#include <Windows.h>
#else
#include <sys/time.h>
#include <cstring>
#endif 
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <assert.h>
#include "utils_time.h"
#include "Asset.h"

void __Asset::reset() {
	this->current_index = 0;
	this->streaming = false;
}
bool __Asset::is_last_view() {
	return this->current_index == (this->AM.N);
}
void __Asset::_register_header(std::string header, unsigned int column_index){
	this->headers[header] = column_index;
}

void __Asset::_set_asset_frictions(double _slippage, double _spread_commission){
	this->slippage = _slippage;
	this->spread_commission = _spread_commission;
}

void __Asset::_set_asset_warmup(unsigned int minimum_warmup){
	this->minimum_warmup = minimum_warmup;
}

void __Asset::_load_format(__AssetDataFormat &format){
	this->open_col_bid = format.open_col_bid;
	this->open_col_ask = format.open_col_ask;
	this->close_col_bid = format.close_col_bid;
	this->close_col_ask = format.close_col_ask;
	this->digit_datetime_format = format.digit_datetime_format;
}

void __Asset::_load_from_pointer(double *datetime_index, double *data, size_t rows, size_t columns) {
	size_t size = rows * columns;

	double whole, fractional;
	long tv_sec;
	int tv_usec;
	timeval tv;
	size_t i;
	for (i = 0; i < rows; i++) {
		fractional = std::modf(datetime_index[i], &whole);
		tv_sec = static_cast<long>(whole);
		fractional *= 1e6;
		tv_usec = static_cast<long>(fractional);
		tv = { tv_sec, tv_usec};
		this->datetime_index.emplace_back(tv);
		this->epoch_index.emplace_back(datetime_index[i]);
		
	}
	for(i = 0; i < size; i++){
		this->AM.data.emplace_back(data[i]);
	}
	this->AM.set_size(rows, columns);
}

void __Asset::_load_from_csv(const char *file_name)
{
	FILE* fp;
	char line_buffer[1024];
	fp = fopen(file_name, "r");
	if (fp == NULL) {
		perror("Error");
		exit(1);
	}
	char *token;
	fgets(line_buffer, 1024, fp);
	token = strtok(line_buffer, ",");
	int i = 0;
	while (token != NULL)
	{
		this->headers[token] = i;
		token = strtok(NULL, ",");
		i++;
	}
	timeval tv;
	while (fgets(line_buffer, 1024, fp) != NULL) {
		token = strtok(line_buffer, ",");
		string_to_timeval(&tv, token, this->digit_datetime_format);
		this->datetime_index.emplace_back(tv);
		double _time = tv.tv_sec + tv.tv_usec / 1e6;
		this->epoch_index.push_back(_time);

		token = strtok(NULL, ",");
		while (token != NULL)
		{
			this->AM.data.emplace_back(atof(token));
			token = strtok(NULL, ",");
		}
	}
	fclose(fp);
	this->AM.set_size(this->AM.data.size() / (this->headers.size() - 1), this->headers.size() - 1);
}
void __Asset::print_data()
{
	size_t i = 0;
	for ( const auto &myPair : this->headers ) {
        std::cout << myPair.first;
				if (i < AM.M) {
			std::cout << ", ";
		}
		else if (i == AM.M) {
			std::cout << "\n";
		}
		i++;
    }
	for (size_t i = 0; i < this->AM.N; i++) {
		char buf[28]{};
		timeval_to_char_array(&this->datetime_index[i], buf, sizeof(buf));
		std::cout << buf << ", ";
		size_t idx = this->AM.row_start(i);
		for (size_t j = 0; j < this->AM.M; j++) {
			std::cout << this->AM.data[idx];
			if (j < this->AM.M - 1) {
				std::cout << ", ";
			}
			else if (j == this->AM.M - 1) {
				std::cout << "\n";
			}
			idx++;
		}
	}
}
void * CreateAssetPtr(unsigned int asset_id, unsigned int exchange_id){
	 __AssetDataFormat format = __AssetDataFormat();
	return new __Asset(
		asset_id, 
		format,
		0,
		exchange_id
	);
}
void DeleteAssetPtr(void *ptr){
	__Asset *__asset_ref = reinterpret_cast<__Asset *>(ptr);
	delete __asset_ref;
}
bool AssetCompare(void *asset_ptr1, void *asset_ptr2) {
	return asset_ptr1 == asset_ptr2;
}
int TestAssetPtr(void *ptr){
	__Asset *__asset_ref = reinterpret_cast<__Asset *>(ptr);
	return __asset_ref->current_index + 4;
}
void load_from_csv(void *ptr, const char* file_name) {
	__Asset *__asset_ref = reinterpret_cast<__Asset *>(ptr);
	__asset_ref->_load_from_csv(file_name);
}
void load_from_pointer(void *ptr, double *datetime_index, double *data, size_t rows, size_t columns){
	__Asset *__asset_ref = reinterpret_cast<__Asset *>(ptr);
	__asset_ref->_load_from_pointer(datetime_index,data,rows,columns);
}
void register_header(void *ptr, const char *header, unsigned int column_index){
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(ptr);
	std::string column_name(header);
	__asset_ref->_register_header(column_name, column_index);
}
double* get_data(void *ptr) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(ptr);
	return &__asset_ref->AM.data[0];
}
size_t rows(void *ptr) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(ptr);
	return __asset_ref->AM.N;
}
size_t columns(void *ptr) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(ptr);
	return __asset_ref->AM.M;
}
void set_format(void *ptr, const char * dformat,
			size_t _open_col_bid,
			size_t _open_col_ask,
			size_t _close_col_bid,
			size_t _close_col_ask,
			ASSET_FREQUENCY asset_frequency) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(ptr);
	__AssetDataFormat format(dformat, 
		_open_col_bid, 
		_open_col_ask,
		_close_col_bid,
		_close_col_ask,
		asset_frequency);
	__asset_ref->_load_format(format);
}
void set_asset_frictions(void *asset_ptr, double _slippage, double _spread_commission) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(asset_ptr);
	__asset_ref->_set_asset_frictions(_slippage, _spread_commission);
}
void set_asset_warmup(void *asset_ptr, unsigned int minimum_warmup) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(asset_ptr);
	__asset_ref->_set_asset_warmup(minimum_warmup);
}
long * get_asset_index(void *asset_ptr) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(asset_ptr);
	return __asset_ref->epoch_index.data();
}
double * get_asset_data(void *asset_ptr) {
	__Asset * __asset_ref = reinterpret_cast<__Asset *>(asset_ptr);
	return __asset_ref->AM.data.data();
}