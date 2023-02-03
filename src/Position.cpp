#ifdef _WIN32
#include "pch.h"
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif 
#include <iostream>
#include <string>
#include "Position.h"
#include "Risk.h"

void Position::to_struct(PositionStruct &position_struct){
	position_struct.average_price = this->average_price;
	position_struct.close_price = this->close_price;
	position_struct.last_price = this->last_price;

	position_struct.bars_held = this->bars_held;
	position_struct.bars_since_change = this->bars_since_change;
	position_struct.units = this->units;

	position_struct.position_id = this->position_id;
	position_struct.asset_id = this->asset_id;
	position_struct.exchange_id = this->exchange_id;
	position_struct.account_id = this->account_id;
	position_struct.strategy_id = this->strategy_id;

	position_struct.position_create_time = this->position_create_time.tv_sec + this->position_create_time.tv_usec/1e6;
	position_struct.position_close_time = this->position_close_time.tv_sec + this->position_close_time.tv_usec / 1e6;
	
	position_struct.realized_pl = this->realized_pl;
	position_struct.unrealized_pl = this->unrealized_pl;
}

Position::Position(unsigned int position_id, unsigned int asset_id, double units, double average_price, timeval position_create_time,
			unsigned int exchange_id,
			unsigned int account_id,
			unsigned int strategy_id) {
	this->position_create_time = position_create_time;
	this->units = units;
	this->average_price = average_price;

	this->position_id = position_id;
	this->asset_id = asset_id;
	this->exchange_id = exchange_id;
	this->account_id = account_id;
	this->strategy_id = strategy_id;

	this->child_trades[this->trade_counter] = Trade(this, this->trade_counter, units, average_price, position_create_time);
	this->trade_counter++;
}

void Position::increase(double market_price, double _units, unsigned int trade_id) {
	double new_units = abs(this->units) + abs(_units);
	this->average_price = ((abs(this->units)*this->average_price) + (abs(_units)*market_price)) / new_units;
	this->units += _units;
	this->bars_since_change = 0;

	//test to see if position is new
	if(this->child_trades.count(trade_id) == 0){
		this->child_trades[this->trade_counter] = Trade(this, this->trade_counter, _units, average_price, position_create_time);
		this->trade_counter++;

	}
	else{
		Trade& existing_trade = this->child_trades[trade_id];
		existing_trade.increase(market_price, _units);
	}
}

void Position::reduce(double market_price, double _units, unsigned int trade_id) {
	this->realized_pl += abs(_units) * (market_price - this->average_price);
	this->units -= abs(_units);
	this->bars_since_change = 0;

	//test to see if position is new
	if(this->child_trades.count(trade_id) == 0){
		this->child_trades[this->trade_counter] = Trade(this, this->trade_counter, _units, average_price, position_create_time);
		this->trade_counter++;
	}
	else{
		Trade& existing_trade = this->child_trades[trade_id];
		existing_trade.reduce(market_price, _units);
	}
}

void Position::close(double close_price, timeval position_close_time, unsigned int trade_id) {
	this->is_open = false;
	this->close_price = close_price;
	this->position_close_time = position_close_time;
	this->realized_pl += this->units * (close_price - this->average_price);
	this->unrealized_pl = 0;

	Trade& existing_trade = this->child_trades[trade_id];
	existing_trade.close(close_price, position_close_time);
}

double Position::liquidation_value() {
	return this->units * this->last_price;
}

double Position::beta_dollars(const __Asset *benchmark, unsigned int n){
	const __Asset * _asset = this->asset;
    double beta = Risk::beta<double>(_asset, benchmark, n);
    return beta * this->units * this->last_price;
}

void Trade::increase(double market_price, double _units) {
	double new_units = abs(this->units) + abs(_units);
	this->average_price = ((abs(this->units)*this->average_price) + (abs(_units)*market_price)) / new_units;
	this->units += _units;
	this->bars_since_change = 0;
}

void Trade::reduce(double market_price, double _units) {
	this->realized_pl += abs(_units) * (market_price - this->average_price);
	this->units -= abs(_units);
	this->bars_since_change = 0;
}

void Trade::close(double close_price, timeval position_close_time) {
	this->is_open = false;
	this->close_price = close_price;
	this->trade_close_time = position_close_time;
	this->realized_pl += this->units * (close_price - this->average_price);
	this->unrealized_pl = 0;
}