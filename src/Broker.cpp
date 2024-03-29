#ifdef _WIN32
#include "pch.h"
#include <Windows.h>
#else
#include <sys/time.h>
#include <cstring>
#endif 
#include <deque>
#include <unordered_map>
#include <memory>
#include "utils_vector.h"
#include "Order.h"
#include "Position.h"
#include "Asset.h"
#include "Exchange.h"
#include "Broker.h"

void __Broker::reset() {
	this->order_history.clear();
	this->position_history.clear();
	this->trade_history.clear();

	memset(&this->cash_history[0], 0, this->cash_history.size() * sizeof this->cash_history[0]);
	memset(&this->nlv_history[0], 0, this->nlv_history.size() * sizeof this->nlv_history[0]);
	this->position_counter = 0;
	this->order_counter = 0;
	this->current_index = 0;

	for(auto & pair : this->accounts){
		auto & account = pair.second;
		account->reset();
	}
}

void __Broker::clean_up(){
	size_t history_size = this->nlv_history.size();
	size_t position_count = this->position_history.size();

	this->perfomance.time_in_market /= history_size;
	this->perfomance.average_time_in_market /= position_count;

	this->perfomance.winrate /= position_count;
}

void __Broker::set_commission(double commission){
	this->commission = commission;
	if(this->commission > 0){
		this->has_commission = true;
	}
}

void __Broker::build(){

	if(this->debug){
		printf("BUILDING BROKER\n");
		printf("BUILDING %lu EXCHANGES\n", this->exchanges.size());
	}

	size_t size;
	std::vector<long> epoch_index;
	if(this->exchanges.size() > 1){
		for(unsigned int i = 0; i < this->exchanges.size(); i++){
			if(i == 0){
				epoch_index = this->exchanges[i]->epoch_index;
			}
			else{
				epoch_index = getUnion(epoch_index, this->exchanges[i]->epoch_index);
			}
		}
		size = epoch_index.size();
	}
	else if(this->exchanges.size() == 1) {
		size = this->exchanges[0]->datetime_index.size();
	}
	else{
		std::cerr << "NO EXCHANGES TO BUILD" << std::endl;
		return;
	}

	this->cash_history.resize(size,0);
	this->nlv_history.resize(size,0);
	this->margin_history.resize(size,0);

	for(auto & pair : this->accounts){
		auto & account = pair.second;
		account->cash_history.resize(size,0);
		account->nlv_history.resize(size,0);
	}

	if(this->debug){
		printf("BROKER BUILT\n");
	}
}

void __Broker::_broker_register_exchange(__Exchange* exchange_ptr){
	if(this->debug){printf("REGISTERING NEW EXCHANGE, EXCHANGE_ID: %i\n",exchange_ptr->exchange_id);}
	this->exchanges[exchange_ptr->exchange_id] = exchange_ptr;
}

void __Broker::_broker_register_account(__Account* account_ptr){
	if(this->debug){printf("REGISTERING NEW ACCOUNT, ACCOUNT_ID: %i\n", account_ptr->account_id);}
	this->accounts[account_ptr->account_id] = account_ptr;
	account_ptr->broker = this;
	account_ptr->set_margin(this->margin);
}

void __Broker::analyze_step() {
	if(this->debug){
		printf("ANALYZING STEP\n");
	}

	bool eval = false;
	for(auto & pair : this->accounts){
		auto & account = pair.second;
		this->cash_history[this->current_index] += account->cash;
		this->nlv_history[this->current_index] += account->net_liquidation_value;
		this->margin_history[this->current_index] += account->margin_loan;

		account->cash_history[this->current_index] = account->cash;
		account->nlv_history[this->current_index] = account->net_liquidation_value;

		if((account->portfolio.size() > 0) & eval){
			this->perfomance.time_in_market++;
			eval = true;
		}
	}
	this->current_index++;

	if(this->debug){
		printf("FINISHED ANALYZING STEP\n");
	}
}

double __Broker::get_net_liquidation_value() {
	double nlv = 0;
	for(auto & pair : this->accounts){
		__Account* account = pair.second;
		for (auto & position : account->portfolio) {
			nlv += position.second->liquidation_value();
		}
		nlv -= account->margin_loan;
	}
	return nlv;
}

void __Broker::margin_adjustment(std::unique_ptr<Position> &position, double market_price){
	double margin_req_mid;
	if(position->units < 0){
		margin_req_mid = this->short_margin_req;
	}
	else{
		margin_req_mid = this->margin_req;
	}
	double collateral = 0;
	for(auto & trade_pair : position->child_trades){
		auto &trade = trade_pair.second;
		trade.collateral = abs(margin_req_mid * trade.units * market_price);
		collateral += trade.collateral;
	}
	position->collateral = collateral;
}

void __Broker::margin_on_increase(std::unique_ptr<Position> &new_position, std::unique_ptr<Order> &order){
		//set the appropriate margin requirment for the position
		double margin_req_mid;
		if(new_position->units < 0){
			margin_req_mid = this->short_margin_req;
		}
		else{
			margin_req_mid = this->margin_req;
		}
		double order_fill_price = order->fill_price;
		//if is margin account, calculate the colateral needed to maintain the trade and the 
		//margin loan provided by the broker. Subtract collateral from the cash available.
		double collateral = abs(margin_req_mid * order->units*order_fill_price);
		double loan = abs((1-margin_req_mid) * order->units*order_fill_price);

		auto & account = this->accounts[new_position->account_id];
		//remove collateral required for the position from the cash in the account
		account->cash -= collateral;

		//if the position is short, credit the account with the cash from the sale of borrowed securities
		if(new_position->units < 0){
			account->cash -= order->units*order_fill_price;
		}
		account->margin_loan += loan;

		new_position->collateral += collateral;
		new_position->margin_loan += loan;
}

void __Broker::margin_on_reduce(std::unique_ptr<Position> &existing_position, double order_fill_price, double _units){
	double pct_reduce = abs(_units/existing_position->units);
	double collateral_free = existing_position->collateral*pct_reduce;
	auto & account = this->accounts[existing_position->account_id];

	//free the collateral from the position and remove the margin loan
	account->cash += collateral_free;
	existing_position->collateral -= collateral_free;
	
	account->margin_loan -= (existing_position->margin_loan*pct_reduce);

	//if closing a short position have to buy back the position at the filled price
	if(existing_position->units < 0){
		account->cash += _units * order_fill_price;
	}
}

void __Broker::open_position(std::unique_ptr<Order> &order) {
	if (this->logging) {
		printf("BROKER: OPENING NEW POSITION: POSITION_ID: %u\n", this->position_counter);
	}
	double order_fill_price = order->fill_price;

	//build new position from the order informtion
	std::unique_ptr<Position> new_position(new Position(
		this->position_counter,
		order->asset_id,
		order->units,
		order_fill_price,
		order->order_fill_time,
		order->exchange_id,
		order->account_id,
		order->strategy_id
	));
	order->trade_id = 0;

	__Exchange *exchange = this->exchanges[order->exchange_id];
	 new_position->asset = &exchange->market[order->asset_id];

	//adjust account's cash and margin balance accoringly
	__Account* account = this->accounts[order->account_id];
	if(!this->margin){
		account->cash -= order->units*order_fill_price;
	}
	else {
		this->margin_on_increase(new_position, order);
	}

	//reflect any commissions on the account's cash 
	if(this->has_commission){
		account->cash -= this->commission;
		this->total_commission += this->commission;
	}
	this->position_counter++;

	#ifdef CHECK_POSITION_OPEN
	POSITION_CHECK_OPEN position_check = this->check_position_open(new_position);
	if(position_check != VALID_POSITION){
		throw std::runtime_error("New position check failed. Error code: " + std::to_string(position_check));
	}
	#endif

	if (this->logging) {
		log_open_position(new_position);
		log_open_trade(new_position->child_trades[order->trade_id]);
	}

	//insert the new postion into the account's portfolio
	account->portfolio[order->asset_id] = std::move(new_position);
}

void __Broker::close_position(std::unique_ptr<Position> &existing_position, std::unique_ptr<Order>& order_filled) {
	//note: this function does not remove the position from the portfolio so this should not
	//be called directly in order to close a position. Close position through a appropriate order.
	
	double order_fill_price = order_filled->fill_price;
	timeval order_fill_time = order_filled->order_fill_time;
	//close the position at the order fill price and order fill time
	existing_position->close(order_fill_price, order_fill_time);
	
	//calculate stats from the position 
	__Account* account = this->accounts[existing_position->account_id];
	account->realized_pl += existing_position->realized_pl;
	this->perfomance.average_time_in_market += existing_position->position_close_time - existing_position->position_create_time;
	if(existing_position->realized_pl > 0){this->perfomance.winrate++;}

	//if commissions are turned on subtract from account's cash
	if(this->has_commission){
		account->cash -= this->commission;
		this->total_commission += this->commission;
	}

	//adjust the accounts cash and margin status if needed
	if(this->margin){
		this->margin_on_reduce(existing_position, order_fill_price, existing_position->units);
	}
	else{
		account->cash += existing_position->units * order_fill_price;
	}

	//close any child orders:
	for(auto & order_id : existing_position->child_order_ids){
		this->cancel_order(order_id);
	}

	if (this->logging) { log_close_position(existing_position); }

	//move child trades to the trade history to keep track of historical trades
	for (auto it = existing_position->child_trades.begin(); it != existing_position->child_trades.end();) {
		Trade &trade_ref =  it->second;
		if(trade_ref.is_open){trade_ref.close(order_filled->fill_price, order_filled->order_fill_time);}
		if(this->logging){log_close_trade(trade_ref);}

		auto trade = std::move(existing_position->child_trades[trade_ref.trade_id]);
		this->trade_history.push_back(trade);
		it = existing_position->child_trades.erase(it);
	}

	//move the position to the position history vector to keep track of historical positions
	this->position_history.push_back(std::move(existing_position));
}

void __Broker::close_positions(int account_id, int asset_id){
	//TODO
}

void __Broker::reduce_position(std::unique_ptr<Position> &existing_position, std::unique_ptr<Order>& order) {
	if(this->margin){
		this->margin_on_reduce(existing_position, order->fill_price, order->units);
	}
	else{
		auto & account = this->accounts[existing_position->account_id];
		account->cash += abs(order->units) * order->fill_price;
	}

	Trade &trade_ref = existing_position->reduce(order->fill_price,order->units,order->order_fill_time,order->trade_id);
	order->trade_id = trade_ref.trade_id;

	//if trade has been closed by the order move it to history
	if(trade_ref.is_open == false){
		if(this->logging){log_close_trade(trade_ref);}
		this->trade_history.push_back(std::move(trade_ref));
		existing_position->child_trades.erase(trade_ref.trade_id);
	}
	else if(trade_ref.bars_held == 0 && this->logging){log_open_trade(trade_ref);}
}

void __Broker::increase_position(std::unique_ptr<Position> &existing_position, std::unique_ptr<Order>& order) {
	if(this->margin){
		this->margin_on_increase(existing_position, order);
	}
	else{
		auto & account = this->accounts[existing_position->account_id];
		account->cash -= order->units * order->fill_price;
	}

	Trade &trade_ref = existing_position->increase(order->fill_price,order->units,order->order_fill_time,order->trade_id);
	order->trade_id = trade_ref.trade_id;

	//if trade has been closed by the order move it to history
	if(trade_ref.is_open == false){
		if(this->logging){log_close_trade(trade_ref);}
		this->trade_history.push_back(std::move(existing_position->child_trades[trade_ref.trade_id]));
		existing_position->child_trades.erase(trade_ref.trade_id);
	}
	else if(trade_ref.bars_held == 0 && this->logging){log_open_trade(trade_ref);}
}

void __Broker::log_canceled_orders(std::vector<std::unique_ptr<Order>> cleared_orders) {
	for (auto& order : cleared_orders) {
		this->order_history.push_back(std::move(order));
	}
}

bool __Broker::cancel_order(unsigned int order_id_cancel) {
	for(auto& pair : this->exchanges){
		__Exchange *exchange = pair.second;
		for (auto& order : exchange->orders) {
			if(order->order_id == order_id_cancel){
				//remove the order from the exchange
				std::unique_ptr<Order> canceled_order = exchange->cancel_order(order);
				canceled_order->order_state = CANCELED;
				
				//if the order is a child remove it from it's parent child order container
				this->remove_child_order(canceled_order);
				this->order_history.push_back(std::move(canceled_order));
				return true;
			}
		}
	}
	return false;
}

bool __Broker::cancel_orders(unsigned int asset_id) {
	for(auto & pair : this->exchanges){
		__Exchange *exchange = pair.second;
		for (auto& order : exchange->orders) {
			if (order->asset_id != asset_id) { continue; }
			if (!this->cancel_order(order->order_id)) {
				return false;
			}
		}
	}
	return true;
}

void __Broker::clear_child_orders(Position& existing_position) {
	for(auto const& pair : this->exchanges){
		__Exchange *exchange = pair.second;
		for (auto& order : exchange->orders) {
			if (order->order_type == STOP_LOSS_ORDER || order->order_type == TAKE_PROFIT_ORDER) {
				StopLossOrder* stop_loss = static_cast <StopLossOrder*>(order.get());
				if (*stop_loss->order_parent.member.parent_position == existing_position) {
					exchange->cancel_order(order);
				}
			}
		}
	}
}

MARGIN_CHECK __Broker::check_margin() noexcept{
	if(!margin){
		return VALID_ACCOUNT_STATUS;
	}

	double margin = 0;
	for(const auto & pair : this->accounts){
		margin += pair.second->net_liquidation_value;
		//check to see if any account has negative cash balance
		if(pair.second->cash < 0){
			return MARGIN_CALL;
		}
	}
	//check to see if nlv is below broker's margin account requirment 
	if(margin < this->minimum_nlv){
		return NLV_BELOW_BROKER_REQ;
	}
	return VALID_ACCOUNT_STATUS;
}

#ifdef CHECK_ORDER

OrderCheck __Broker::check_position_open(const std::unique_ptr<Order>& new_order){
	double new_position_units = new_order->units;
	unsigned int new_position_asset_id = new_order->asset_id;

	//check to see if this order will open a new position
	auto &account = this->accounts[new_order->account_id];
	if(account->portfolio.count(new_position_asset_id) > 0){
		return VALID_ORDER;
	}

	//check to see if there exists a position in a different account with a different direction (short/long)
	//cannnot have two positions with the same asset with different directions
	for (const auto & account_pair : this->accounts){
		__Account *account = account_pair.second;
		for(const auto & position_pair : account->portfolio){
			if(position_pair.second->asset_id != new_position_asset_id){
				continue;
			}
			if(position_pair.second->units * new_position_units < 0 ){
				return INVALID_NEW_POSITION_SIDE;
			}
			else{
				return VALID_ORDER;
			}
		}
	}
	return VALID_ORDER;
}

OrderCheck __Broker::check_stop_loss_order(const StopLossOrder* stop_loss_order) {
	OrderParent parent = stop_loss_order->order_parent;
	if (parent.type == ORDER) {
		Order* parent_order = parent.member.parent_order;
		if (parent_order->asset_id != stop_loss_order->asset_id) { return INVALID_ASSET; }
		if (parent_order->order_type > 1) { return INVALID_PARENT_ORDER; }
		if (parent_order->units*stop_loss_order->units > 0) { return INVALID_ORDER_SIDE; }
	}
	else {
		Position* parent_position = parent.member.parent_position;
		if (parent_position->asset_id != stop_loss_order->asset_id) { return INVALID_ASSET; }
		if (parent_position->units*stop_loss_order->units > 0) { return INVALID_ORDER_SIDE; }
	}
	return VALID_ORDER;
}

OrderCheck __Broker::check_take_profit_order(const TakeProfitOrder* take_profit_order){
	OrderParent parent = take_profit_order->order_parent;
	if (parent.type == ORDER) {
		Order* parent_order = parent.member.parent_order;
		if (parent_order->asset_id != take_profit_order->asset_id) { return INVALID_ASSET; }
		if (parent_order->order_type > 1) { return INVALID_PARENT_ORDER; }
		if (parent_order->units*take_profit_order->units > 0) { return INVALID_ORDER_SIDE; }
	}
	else {
		Position* parent_position = parent.member.parent_position;
		if (parent_position->asset_id != take_profit_order->asset_id) { return INVALID_ASSET; }
		if (parent_position->units*take_profit_order->units > 0) { return INVALID_ORDER_SIDE; }
	}
	return VALID_ORDER;
}

OrderCheck __Broker::check_market_order(const MarketOrder* market_order) {

	if(this->debug){
		printf("CHECKING MARKET ORDER, ORDER_ID: %i\n", market_order->order_id);
	}

	unsigned int asset_id = market_order->asset_id;
	double units = market_order->units;

	__Exchange *exchange = this->exchanges[market_order->exchange_id];
	auto &account = this->accounts[market_order->account_id];

	double market_price = exchange->_get_market_price(asset_id, units);

	if(!this->_position_exists(asset_id)){
		auto & account = this->accounts[market_order->account_id];
		if(market_order->units * market_price > account->cash){
			return INVALID_ORDER_COLLATERAL;
		};
	}
	else if (account->portfolio[asset_id]->units * units > 0){
		auto & account = this->accounts[market_order->account_id];
		if(market_order->units * market_price > account->cash){
			return INVALID_ORDER_COLLATERAL;
		};
	}
	return VALID_ORDER;
}

OrderCheck __Broker::check_order(const std::unique_ptr<Order>& new_order) {

	if(this->debug){
		printf("CHECKING ORDER, ORDER_ID: %u, EXCHANGE_ID: %u\n", new_order->order_id, new_order->exchange_id);
	}
	OrderCheck order_code;

	//check to see if the asset exsists on the exchange
	__Exchange *exchange = this->exchanges[new_order->exchange_id];
 
	//check to see if exchange contains the asset
	if (exchange->market.count(new_order->asset_id) == 0){
		return INVALID_ASSET; 
	}
	//check to see if the order has valid units
	if (new_order->units == 0){
		return INVALID_ORDER_UNITS; 
	}

	//if order opens new position make sure it is valid new position side
	order_code = check_position_open(new_order);
	if(order_code!= VALID_ORDER){
		return order_code;
	}

	switch (new_order->order_type) {
		case MARKET_ORDER: {
			MarketOrder* market_order = static_cast <MarketOrder*>(new_order.get());
			order_code = check_market_order(market_order);
			break;
		}
		case LIMIT_ORDER: {
			order_code = VALID_ORDER;
			break;
		}
		case STOP_LOSS_ORDER: {
			StopLossOrder* stop_loss_order = static_cast <StopLossOrder*>(new_order.get());
			order_code = check_stop_loss_order(stop_loss_order);
			break;
		}
		case TAKE_PROFIT_ORDER: {
			TakeProfitOrder* take_profit_order = static_cast <TakeProfitOrder*>(new_order.get());
			order_code = check_take_profit_order(take_profit_order);
			break;
		}
	}
	if (order_code != VALID_ORDER) { return order_code; }
	for (auto& order_on_fill : new_order->orders_on_fill) {
		this->check_order(order_on_fill);
	}
	if(this->debug){
		printf("CHECKING ORDER STATE, ORDER_ID: %u, ORDER_CHECK: %u\n", new_order->order_id, order_code);
	}
	return order_code;
}
#endif
void __Broker::send_order(std::unique_ptr<Order> new_order,OrderResponse *order_response) {

	if(this->debug){
		printf("SENDING ORDER %i TO EXCHANGE %i\n", new_order->order_id, new_order->exchange_id);
	}

	new_order->order_state = ACCEPETED;
	__Exchange* exchange = this->exchanges[new_order->exchange_id];
	new_order->order_create_time = exchange->current_time;
	exchange->place_order(std::move(new_order));

	order_response->order_state = ACCEPETED;
	this->order_counter++;
}

void __Broker::_place_market_order(OrderResponse *order_response, unsigned int asset_id, double units, bool cheat_on_close,
				unsigned int exchange_id,
				unsigned int strategy_id,
				unsigned int account_id,
				int trade_id) {
	
	if(this->debug){
		printf("PLACING MARKER ORDER, ASSET_ID: %i TO EXCHANGE_ID: %i\n",asset_id, exchange_id);
	}
	
	std::unique_ptr<Order> order(new MarketOrder(
		asset_id,
		units,
		cheat_on_close,
		exchange_id,
		account_id,
		trade_id
	));
	order->strategy_id = strategy_id;
	order->order_id = this->order_counter;

#ifdef CHECK_ORDER
	OrderCheck order_check = check_order(order);
	if (order_check!= VALID_ORDER) {
		order->order_state = BROKER_REJECTED;
		this->order_history.push_back(std::move(order));

		order_response->order_state = BROKER_REJECTED;
		order_response->order_check = order_check;

		throw std::runtime_error("Order check failed: Error code: " + std::to_string(order_check));
	}
#endif
	this->send_order(std::move(order), order_response);
}

void __Broker::_place_limit_order(OrderResponse *order_response, unsigned int asset_id, double units, double limit, bool cheat_on_close,
				unsigned int exchange_id, 
				unsigned int strategy_id, 
				unsigned int account_id,
				int trade_id) {
	std::unique_ptr<Order> order(new LimitOrder(
		asset_id,
		units,
		limit,
		cheat_on_close,
		exchange_id,
		account_id,
		trade_id
	));
	order->strategy_id = strategy_id;
	order->order_id = this->order_counter;

#ifdef CHECK_ORDER
	OrderCheck order_check = check_order(order);
	if (order_check != VALID_ORDER) {
		order->order_state = BROKER_REJECTED;
		this->order_history.push_back(std::move(order));

		order_response->order_state = BROKER_REJECTED;
		order_response->order_check = order_check;
		throw std::runtime_error("Order check failed: Error code: " + std::to_string(order_check));
	}
#endif
	this->send_order(std::move(order), order_response);
}

void __Broker::place_stoploss_order(Position* parent, OrderResponse *order_response, double units, double stop_loss, 
				bool cheat_on_close,
				bool limit_pct,
				int trade_id) {
	std::unique_ptr<Order> order(new StopLossOrder(
		parent,
		units,
		stop_loss,
		cheat_on_close,
		limit_pct,
		parent->account_id,
		trade_id
	));
	order->order_id = this->order_counter;

#ifdef CHECK_ORDER
	OrderCheck order_check = check_order(order);
	if (order_check != VALID_ORDER) {
		order->order_state = BROKER_REJECTED;
		this->order_history.push_back(std::move(order));

		order_response->order_state = BROKER_REJECTED;
		order_response->order_check = order_check;
		return;
	}
#endif
	parent->child_order_ids.push_back(this->order_counter);
	this->send_order(std::move(order), order_response);
}

void __Broker::remove_child_order(std::unique_ptr<Order>& child_order){
	if(child_order->order_type == STOP_LOSS_ORDER){
		StopLossOrder* stop_loss_order = static_cast <StopLossOrder*>(child_order.get());
		
		//parent is a position
		if(stop_loss_order->order_parent.type == POSITION){

			Position* parent_position = stop_loss_order->order_parent.member.parent_position;
			auto & ids =  parent_position->child_order_ids;
			for(unsigned int i = 0; i < ids.size(); i++){
				if(ids[i] == child_order->order_id){
					ids.erase(ids.begin() + i);
					return;
				}
			}
		}
	}
	else if(child_order->order_type == TAKE_PROFIT_ORDER){
		//TakeProfitOrder* stop_loss_order = static_cast <TakeProfitOrder*>(child_order.get());
		return;
	}
	else{
		return;
	}
}

void __Broker::process_filled_orders(std::vector<std::unique_ptr<Order>> orders_filled) {
	for (auto& order : orders_filled) {
		auto & account = this->accounts[order->account_id];

		if(this->debug){
			printf("BROKER: ORDER_ID: %i FILLED\n", order->order_id);
		}

		auto order_type = order->order_type;
		if(order_type == STOP_LOSS_ORDER || order_type == TAKE_PROFIT_ORDER){
			remove_child_order(order);
		}

		//no position exists, create new open position
		if (!this->_position_exists(order->asset_id)) {
			this->open_position(order);
		}
		else {
			std::unique_ptr<Position> &existing_position = account->portfolio[order->asset_id];
			//sum of existing position units and order units is 0. Close existing position
			if (abs(existing_position->units + order->units) < POSITION_REVERSE_TOL) {
				this->close_position(existing_position, order);
				account->portfolio.erase(order->asset_id);
			}
			//order is same direction as existing position. Increase existing position
			else if (existing_position->units * order->units > 0) {
				this->increase_position(existing_position, order);
			}
			//order is in opposite direction as existing position. Reduce existing position
			else {
				if(abs(order->units) < abs(existing_position->units)){
					this->reduce_position(existing_position, order);
				}
				else{
					order->units = order->units + existing_position->units;
					this->close_position(existing_position, order);
					account->portfolio.erase(order->asset_id);
					this->open_position(order);
				}
			}
			//if(existing_position->units < .0001){
			//	throw std::runtime_error("EXIST POSTION MIN UNITS ERROR\n");
			//}
		}
		this->order_history.push_back(std::move(order));
	}
}
std::deque<std::unique_ptr<Order>>& __Broker::open_orders(unsigned int exchange_id) {
	__Exchange* exchange = this->exchanges[exchange_id];
	return exchange->orders;
}
bool __Broker::_position_exists(int asset_id, int account_id) {
	unsigned int uaccount_id = abs(account_id);

	for (const auto & pair : this->accounts){
		auto & account = pair.second;
		//found the assed inside an account
		if(account->portfolio.count(asset_id) > 0){
			//if account_id == -1 searching for position in any account
			if(account_id == -1){
				return true;
			}
			//else make sure the account id is equal to the account id that was passed
			else if (uaccount_id == account->account_id){
				return true;
			}
			return true;
		}
	}
	return false;
}
void __Broker::set_cash(double cash, unsigned int account_id) {
	auto & account = this->accounts[account_id];
	account->cash = cash;
}
void __Broker::log_open_position(std::unique_ptr<Position> &position) {
	memset(this->time, 0, sizeof this->time);
	timeval_to_char_array(&position->position_create_time, this->time, sizeof(this->time));
	printf("BROKER:  %s: POSITION OPENED: asset_id: %i, units: %f, avg_price: %f\n",
		this->time,
		position->asset_id,
		position->units,
		position->average_price
	);
}
void __Broker::log_close_position(std::unique_ptr<Position> &position) {
	memset(this->time, 0, sizeof this->time);
	timeval_to_char_array(&position->position_close_time, this->time, sizeof(this->time));
	printf("BROKER:  %s: POSITION CLOSED: asset_id: %i, units: %f, close_price: %f\n",
		this->time,
		position->asset_id,
		position->units,
		position->close_price
	);
}
void __Broker::log_close_trade(Trade &trade) {
	memset(this->time, 0, sizeof this->time);
	timeval_to_char_array(&trade.trade_close_time, this->time, sizeof(this->time));
	printf("BROKER:  %s: TRADE CLOSED: trade_id: %i, asset_id: %i, units: %f, close_price: %f\n",
		this->time,
		trade.trade_id,
		trade.parent_position->asset_id,
		trade.units,
		trade.close_price
	);
}
void __Broker::log_open_trade(Trade &trade) {
	memset(this->time, 0, sizeof this->time);
	timeval_to_char_array(&trade.trade_create_time, this->time, sizeof(this->time));
	printf("BROKER:  %s: TRADE OPENED: trade_id: %i, asset_id: %i, units: %f, close_price: %f\n",
		this->time,
		trade.trade_id,
		trade.parent_position->asset_id,
		trade.units,
		trade.average_price
	);
}

void * CreateBrokerPtr(void *exchange_ptr,bool logging, bool margin, bool debug) {
	__Exchange *__exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	return new __Broker(__exchange_ref, logging, margin, debug);
}
void DeleteBrokerPtr(void *ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(ptr);
	delete __broker_ref;
}
void reset_broker(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__broker_ref->reset();
}
void build_broker(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__broker_ref->build();
}
void broker_set_commission(void* broker_ptr, double commission){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__broker_ref->set_commission(commission);
}
void broker_register_exchange(void *broker_ptr, void *exchange_ptr){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__Exchange *__exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	__broker_ref->_broker_register_exchange(__exchange_ref);
}
void broker_register_account(void *broker_ptr, void * account_ptr){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__Account *__account_ref = static_cast<__Account *>(account_ptr);
	__broker_ref->_broker_register_account(__account_ref);
}
double get_cash(void *broker_ptr, int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	if(account_id == -1){
		double cash = 0;
		for (const auto & pair : __broker_ref->accounts){
			cash += pair.second->cash;
		}
		return cash;
	}
	else{
		unsigned int uid = abs(account_id);
		return __broker_ref->accounts[uid]->cash;
	}
}
double get_nlv(void *broker_ptr, int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	double nlv;
	if(account_id == -1){
		nlv = 0;
		for (const auto & pair : __broker_ref->accounts){
			nlv += pair.second->net_liquidation_value;
		}
	}
	else{
		unsigned int uid = abs(account_id);
		nlv =  __broker_ref->accounts[uid]->net_liquidation_value;
	}
	if(nlv != nlv){
			throw std::runtime_error("net liquidation value is NAN");
	}
	return nlv;
}
double get_unrealized_pl(void *broker_ptr, int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	if(account_id == -1){
		double upl = 0;
		for (const auto & pair : __broker_ref->accounts){
			upl += pair.second->unrealized_pl;
		}
		return upl;
	}
	else{
		unsigned int uid = abs(account_id);
		return __broker_ref->accounts[uid]->unrealized_pl;
	}
}
double get_realized_pl(void *broker_ptr, int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	if(account_id == -1){
		double pl = 0;
		for (const auto & pair : __broker_ref->accounts){
			pl += pair.second->realized_pl;
		}
		return pl;
	}
	else{
		unsigned int uid = abs(account_id);
		return __broker_ref->accounts[uid]->realized_pl;
	}
}

int get_open_order_count(void *broker_ptr){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	int count = 0;
	for (const auto & pair : __broker_ref->exchanges){
		count += pair.second->orders.size();
	}
	return count;
}

int get_open_position_count(void *broker_ptr){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	int count = 0;
	for (const auto & pair : __broker_ref->accounts){
		count += pair.second->portfolio.size();
	}
	return count;
}

int get_open_trade_count(void *broker_ptr){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	int count = 0;
	for (const auto & pair : __broker_ref->accounts){
		const __Account *account = pair.second;
		for(const auto &position_pair : account->portfolio){
			count += position_pair.second->child_trades.size();
		}
	}
	return count;
}

bool position_exists(void *broker_ptr, unsigned int asset_id, int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->_position_exists(asset_id, account_id);
}
void place_market_order(void *broker_ptr, OrderResponse *order_response, unsigned int asset_id, double units, bool cheat_on_close,
			unsigned int exchange_id,
			unsigned int strategy_id,
			unsigned int account_id,
			int trade_id) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	try{
		__broker_ref->_place_market_order(order_response, asset_id, units, cheat_on_close, exchange_id, strategy_id, account_id, trade_id);
	}
	catch (std::runtime_error& e){
		if(__broker_ref->debug){std::cerr << e.what() << std::endl;}
	}
}
void place_limit_order(void *broker_ptr, OrderResponse *order_response,  unsigned int asset_id, double units, double limit, bool cheat_on_close,
			unsigned int exchange_id,
			unsigned int strategy_id,
			unsigned int account_id,
			int trade_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	try{
		__broker_ref->_place_limit_order(order_response, asset_id, units, limit, cheat_on_close, exchange_id, strategy_id, account_id, trade_id);
	}
	catch (std::runtime_error& e){
		if(__broker_ref->debug){std::cerr << e.what() << std::endl;}
	}
}
void position_add_stoploss(void *broker_ptr, OrderResponse *order_response, void * position_ptr, double units, double stop_loss, bool cheat_on_close, bool limit_pct, int trade_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	Position * __position_ref = static_cast<Position *>(position_ptr);
	__broker_ref->place_stoploss_order(
		__position_ref, order_response, units, stop_loss, cheat_on_close, limit_pct, trade_id
	);
}
void order_add_stoploss(void *broker_ptr, OrderResponse *order_response, unsigned int order_id, double units, double stop_loss, unsigned int exchange_id, bool limit_pct){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__Exchange *exchange = __broker_ref->exchanges[exchange_id];
	for (auto& order : exchange->orders){
		if(order->order_id == order_id){
			order->add_stop_loss(stop_loss, units, limit_pct);
			order_response->order_state = ACCEPETED;
			order_response->order_id = __broker_ref->order_counter;
			return;
		}
	}
	order_response->order_state = INVALID_PARENT_ORDER_ID;
}

int get_order_count(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->order_history.size();
}

int get_position_count(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->position_history.size();
}

int get_trade_count(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->trade_history.size();
}

size_t broker_get_history_length(void *broker_ptr){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->nlv_history.size();
}

double * broker_get_nlv_history(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->nlv_history.data();
}

double * broker_get_cash_history(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->cash_history.data();
}

double * broker_get_margin_history(void *broker_ptr) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	return __broker_ref->margin_history.data();
}

void get_order_history(void *broker_ptr, OrderArray *order_history) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	int number_orders = order_history->number_orders;
	for (int i = 0; i < number_orders; i++) {
		OrderStruct &order_struct_ref = *order_history->ORDER_ARRAY[i];
		__broker_ref->order_history[i]->to_struct(order_struct_ref);
	}
}

void get_position_history(void *broker_ptr, PositionArray *position_history) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	int number_positions = position_history->number_positions;
	for (int i = 0; i < number_positions; i++) {
		PositionStruct &position_struct_ref = *position_history->POSITION_ARRAY[i];
		__broker_ref->position_history[i]->to_struct(position_struct_ref);
	}
}

void get_trade_history(void *broker_ptr, TradeArray *trade_history) {
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	int number_trades = trade_history->number_trades;
	for (int i = 0; i < number_trades; i++) {
		TradeStruct &trade_struct_ref = *trade_history->TRADE_ARRAY[i];
		__broker_ref->trade_history[i].to_struct(trade_struct_ref);
	}
}

void get_positions(void *broker_ptr, PositionArray *positions, unsigned int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	auto & account = __broker_ref->accounts[account_id];
	int i = 0;
	for (auto &kvp : account->portfolio){
		PositionStruct &position_struct_ref = *positions->POSITION_ARRAY[i];
		account->portfolio[kvp.first]->to_struct(position_struct_ref);
		i++;
	}
}

void get_trades(void *broker_ptr, TradeArray *trades, int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	int i = 0;
	for (const auto & pair : __broker_ref->accounts){
		for(const auto &position_pair :  pair.second->portfolio){
			for(const auto &trade_pair : position_pair.second->child_trades){
				TradeStruct &trade_struct_ref = *trades->TRADE_ARRAY[i];
				const Trade &trade = trade_pair.second;
				trade.to_struct(trade_struct_ref);
				i++;
			}
		}
	}
}

int get_position_trade_count(void *broker_ptr, unsigned int account_id, unsigned int asset_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__Account *__account_ref = __broker_ref->accounts[account_id];
	std::unique_ptr<Position> &position = __account_ref->portfolio[asset_id];
	return position->child_trades.size();
}

void get_position_trades(void *broker_ptr , TradeArray *trades, unsigned int account_id, unsigned int asset_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__Account *__account_ref = __broker_ref->accounts[account_id];
	std::unique_ptr<Position> &position = __account_ref->portfolio[asset_id];

	int i = 0 ;
	for(auto &trade_pair : position->child_trades){
		TradeStruct &trade_struct_ref = *trades->TRADE_ARRAY[i];
		const Trade &trade = trade_pair.second;
		trade.to_struct(trade_struct_ref);
		i++;
	}
}

void get_position(void *broker_ptr, unsigned int assset_id, PositionStruct *position, unsigned int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	auto & account = __broker_ref->accounts[account_id];
	if(account->portfolio.count(assset_id) == 0){return;}
	account->portfolio[assset_id]->to_struct(*position);
}

void get_orders(void *broker_ptr, OrderArray *orders, unsigned int exchange_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	__Exchange *exchange = __broker_ref->exchanges[exchange_id];
	int number_orders = exchange->orders.size();
	for (int i = 0; i < number_orders; i++) {
		OrderStruct &order_struct_ref = *orders->ORDER_ARRAY[i];
		std::unique_ptr<Order>& open_order = exchange->orders[i];
		order_ptr_to_struct(open_order, order_struct_ref);
	}
}

void * get_position_ptr(void *broker_ptr, unsigned int asset_id, unsigned int account_id){
	__Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
	auto & account = __broker_ref->accounts[account_id];
	return account->portfolio[asset_id].get();
}