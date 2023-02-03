#pragma once
#ifndef POSITION_H // include guard
#define POSITION_H
#ifdef _WIN32
#define POSITION_API __declspec(dllexport)
#else
#define POSITION_API
#endif 
#include "pch.h"
#include <ctime>
#include <vector>
#include "utils_time.h"
#include "Asset.h"

struct PositionStruct {
	double average_price;
	double close_price;
	double last_price;
	double units;

	unsigned int bars_held;
	unsigned int bars_since_change;

	unsigned int position_id;
	unsigned int asset_id;
	unsigned int exchange_id;
	unsigned int account_id;
	unsigned int strategy_id;

	long position_create_time;
	long position_close_time;

	double realized_pl;
	double unrealized_pl;
};

struct PositionArray {
	unsigned int number_positions;
	PositionStruct **POSITION_ARRAY;
};

enum PositionType{
	MAIN_POSITION,
	HEDGE_POSITION
};

class Position;

class Trade 
{
public: 
	bool is_open = true;
	double units = 0;
    double collateral = 0;
    double margin_loan = 0;
    double average_price = 0;
	double close_price;
    double last_price;

    unsigned int trade_id;
	Position* parent_position;
	unsigned int bars_held = 0;
	unsigned int bars_since_change = 0;

	timeval trade_create_time;           /**<the datetime that the trade was created*/
	timeval trade_close_time = MAX_TIME; /**<the datetime that the trade was closed*/

	double unrealized_pl = 0;
	double realized_pl = 0;

	Trade(Position * _parent_position, unsigned int trade_id, double _units, double _average_price, timeval _trade_create_time){
		this->parent_position = _parent_position;
		this->trade_id = trade_id;
		this->units = _units;
		this->average_price = _average_price;
		this->trade_create_time = _trade_create_time;
	}
	Trade() = default;

	void increase(double market_price, double units);
	void reduce(double market_price, double units);
	void close(double close_price, timeval position_close_time);

	inline void evaluate(double market_price, bool update_bars = false) noexcept {
		this->last_price = market_price;
		this->unrealized_pl = this->units * (market_price - this->average_price);
		if(update_bars){
			this->bars_held++;
			this->bars_since_change++;
		}
	}

};

class Position
{
public:

	bool is_open;
	PositionType position_type = MAIN_POSITION;
	unsigned int bars_held = 0;
	unsigned int bars_since_change = 0;

	unsigned int trade_counter = 0; //child trade count tracker

	double units = 0;       /**<number of units of the asset held*/
	double collateral = 0;  /**<collateral broker is holding for the position*/
	double margin_loan = 0; /**<margin loan being provided by broker to maintain the position*/
	double average_price;   /**<average price per unit of the position*/
	double close_price = 0;     /**<the closing price of the position*/
	double last_price;      /**<the last price the the position as evaluated at*/

	__Asset *asset;			  /**<underlying asset of the position*/
	unsigned int position_id; /**<id of the position*/
	unsigned int asset_id;    /**<id of the asset for the order*/
	unsigned int exchange_id; /**<id of the exchange the order was placed on*/
	unsigned int account_id;  /**<id of the account the order was placed for*/
	unsigned int strategy_id; /**<id of the strategy that placed the order*/

	timeval position_create_time;           /**<the datetime that the position was created*/
	timeval position_close_time = MAX_TIME; /**<the datetime that the position was closed*/

	std::vector<unsigned int> child_order_ids;              /**<container for child order ids for the positions*/
	std::unordered_map<unsigned int, Trade> child_trades;   /**<container for the child trades (trade id to trade)*/
	std::vector<std::unique_ptr<Position>*> child_positions;/**<container for linked positions*/

	double unrealized_pl = 0;
	double realized_pl = 0;

	void increase(double market_price, double units, unsigned int trade_id = 0);
	void reduce(double market_price, double units, unsigned int trade_id = 0);
	void close(double close_price, timeval position_close_time, unsigned int trade_id = 0);

	double liquidation_value();
	double beta_dollars(const __Asset *benchmark, unsigned int n);

	void to_struct(PositionStruct &position_struct);

	Position(unsigned int position_id, unsigned int asset_id, double units, double average_price, timeval position_create_time,
			unsigned int exchange_id,
			unsigned int account_id,
			unsigned int strategy_id);

	inline void evaluate(double market_price, bool update_bars = false) noexcept {
		this->last_price = market_price;
		this->unrealized_pl = this->units * (market_price - this->average_price);
		if(update_bars){
			this->bars_held++;
			this->bars_since_change++;
		}
		for(auto & trade_pair : this->child_trades){
			auto &trade = trade_pair.second;
			trade.evaluate(market_price, update_bars);
		}
	}

	friend bool operator==(const Position& lhs, const Position& rhs)
	{
		return &lhs == &rhs;
	}
};

struct PositionHash {
  std::size_t operator() (Position *position) const {
    return std::hash<const Position*>()(position);
  }
};

class Hedge : public Position{
public:
	//map between position pointers and units of the hedge allocated to that position
	std::unordered_map<Position*, double, PositionHash> allocations;
	
	//initilize the Hedge as a regular position
	Hedge(unsigned int position_id, unsigned int asset_id, double units, double average_price, timeval position_create_time,
			unsigned int exchange_id,
			unsigned int account_id,
			unsigned int strategy_id)
		:Position(position_id,asset_id,units,average_price,position_create_time,exchange_id,account_id,strategy_id){
		this->position_type = HEDGE_POSITION;
	}
};

#endif