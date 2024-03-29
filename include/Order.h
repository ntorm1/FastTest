#pragma once
#ifndef ORDER_H // include guard
#define ORDER_H
#include "pch.h"
#include <iostream>
#ifdef _WIN32
#define ORDER_API __declspec(dllexport)
#include <WinSock2.h>
#else
#define ORDER_API
#include <sys/time.h>
#endif 
#include <math.h>
#include <memory>
#include "Position.h"
#include "utils_time.h"
#include <vector>
#define CHECK_ORDER

enum OrderState {
	ACCEPETED,				 //order has been accepted by broker
	OPEN,					 //order is open on the exchange
	FILLED,					 //order has been filled by the exchange
	CANCELED,				 //order has been canceled by strategy
	BROKER_REJECTED,		 //order has been rejected by the broker
	INVALID_PARENT_ORDER_ID, //parent order failed to be found
	BROKER_MERGED			 //broker has merged the order with another 
};

#ifdef CHECK_ORDER
enum OrderCheck {
	VALID_ORDER,
	INVALID_ASSET,
	INVALID_ORDER_SIDE,
	INVALID_ORDER_COLLATERAL,
	INVALID_ORDER_UNITS,
	INVALID_PARENT_ORDER,
	INVALID_NEW_POSITION_SIDE
};
#endif

enum OrderType {
	MARKET_ORDER,
	LIMIT_ORDER,
	STOP_LOSS_ORDER,
	TAKE_PROFIT_ORDER
};

struct OrderResponse {
	unsigned int order_id;
	OrderState order_state;
	OrderCheck order_check;
};

struct OrderStruct {
	OrderType order_type;
	OrderState order_state;
	unsigned int order_id;
	unsigned int asset_id;
	unsigned int strategy_id;
	unsigned int exchange_id;
	double units;
	double fill_price;
	long order_create_time;
	long order_fill_time;
};

struct OrderArray {
	unsigned int number_orders;
	OrderStruct **ORDER_ARRAY;
};

class Order
{
public:
	//track order type (allow for polymorphism so can easily store orders in vecotr)
	OrderType order_type;

	//track state of the order
	OrderState order_state = OPEN;

	//specify wether or not a order can be executed at close of the view it was placed
	bool cheat_on_close = false;

	//once we have seen that the order is not being executed at the current view, it can then
	//be executed at time after
	bool alive = false;

	double units;			  //number of units to buy/sell
	double fill_price;		  //price the order was filled at
	unsigned int order_id;    //unique identifier for the order
	unsigned int asset_id;    //underlying asset for the order
	unsigned int exchange_id; //id of the exchange the order was placed on
	unsigned int strategy_id; //id of the strategy that placed the order
	unsigned int account_id;  //id of the account the order was placed for
	unsigned int trade_id;    //id of the trade that the order was placed for

	timeval order_create_time; //the time the order was placed on the exchange
	timeval order_fill_time;   //the time that the order was filled by the exchange

	std::vector<std::unique_ptr<Order>> orders_on_fill; //container for orders to execute once the parent order as filled

	const char* get_order_type();
	void create(timeval order_create_time);
	void fill(double market_price, timeval fill_time);
	void add_stop_loss(double price, double units = NAN, bool limit_pct = false);

	void to_struct(OrderStruct &order_struct);

	Order(OrderType _OrderType, unsigned int asset_id, double units,
				bool cheat_on_close = false,
				unsigned int exchange_id = 0,
				unsigned int account_id = 0,
				unsigned int trade_id = 0) {
		this->order_type = _OrderType;
		this->units = units;
		this->cheat_on_close = cheat_on_close;

		this->asset_id = asset_id;
		this->exchange_id = exchange_id;
		this->account_id = account_id;
		this->trade_id = trade_id;
	}
	Order() = default;
	virtual ~Order() {};
	Order(const Order&) = delete;
	Order& operator =(const Order&) = delete;

	friend bool operator==(const Order& lhs, const Order& rhs)
	{
		return &lhs == &rhs;
	}
};

void order_ptr_to_struct(std::unique_ptr<Order> &open_order, OrderStruct &order_struct);


class MarketOrder : public Order
{
public:
	MarketOrder(unsigned int asset_id, double units,
			bool cheat_on_close = false,
			unsigned int exchange_id = 0,
			unsigned int account_id = 0,
			unsigned int trade_id = 0)
		: Order(MARKET_ORDER, asset_id, units, cheat_on_close,
			exchange_id,
			account_id,
			trade_id)
	{}
};
class LimitOrder : public Order
{
public:
	double limit;
	LimitOrder(unsigned int asset_id, double units, double limit,
			bool cheat_on_close = false,
			unsigned int exchange_id = 0,
			unsigned int account_id = 0,
			unsigned int trade_id = 0)
		: Order(LIMIT_ORDER, asset_id, units, cheat_on_close,
			exchange_id,
			account_id,
			trade_id) {
		this->limit = limit;
	}
};

enum OrderParentType {
	POSITION = 0,
	ORDER = 1
};

struct OrderParent {
	OrderParentType type;
	union {
		Order* parent_order;
		Position* parent_position;
	} member;
};
class StopLossOrder : public Order
{
public:
	OrderParent order_parent;
	double stop_loss;
	bool limit_pct;
	StopLossOrder(Order *parent_order, double units, double stop_loss,
			bool cheat_on_close = false,
			unsigned int exchange_id = 0,
			bool limit_pct = false,
			unsigned int account_id = 0,
			unsigned int trade_id = 0)
		: Order(STOP_LOSS_ORDER, parent_order->asset_id, units, cheat_on_close,
			exchange_id, 
			account_id,
			trade_id) {
		this->order_parent.member.parent_order = parent_order;
		this->order_parent.type = ORDER;
		this->stop_loss = stop_loss;
		this->limit_pct = limit_pct;
	}
	StopLossOrder(Position *parent_position, double units, double stop_loss,
			bool cheat_on_close = false,
			bool limit_pct = false,
			unsigned int account_id = 0,
			unsigned int trade_id = 0)
		: Order(STOP_LOSS_ORDER, parent_position->asset_id, units, cheat_on_close,
			parent_position->exchange_id,
			account_id,
			trade_id) {
		this->order_parent.member.parent_position = parent_position;
		this->order_parent.type = POSITION;
		this->stop_loss = stop_loss;
		this->limit_pct = limit_pct;

	}
};
class TakeProfitOrder : public Order
{
public:
	OrderParent order_parent;
	double take_profit;
	bool limit_pct;
	TakeProfitOrder(Order *parent_order, double units, double stop_loss,
			bool cheat_on_close = false,
			bool limit_pct = false)
		: Order(TAKE_PROFIT_ORDER, parent_order->asset_id, units, cheat_on_close, parent_order->exchange_id, parent_order->account_id) {
		this->order_parent.member.parent_order = parent_order;
		this->order_parent.type = ORDER;
		this->take_profit = stop_loss;
		this->limit_pct = limit_pct;
	}
	TakeProfitOrder(Position *parent_position, double units, double stop_loss, bool cheat_on_close = false, bool limit_pct = false)
		: Order(TAKE_PROFIT_ORDER, parent_position->asset_id, units, cheat_on_close, parent_position->exchange_id, parent_position->account_id) {
		this->order_parent.member.parent_position = parent_position;
		this->order_parent.type = POSITION;
		this->take_profit = stop_loss;
		this->limit_pct = limit_pct;
	}
};

extern "C" {
	ORDER_API OrderType order_type(void *order_ptr);
	ORDER_API bool order_compare(void *order_ptr1, void *order_ptr2);
}
#endif