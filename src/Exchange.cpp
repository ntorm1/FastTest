#ifdef _WIN32
#include "pch.h"
#include <Windows.h>
#else
#include <sys/time.h>
#include <cstring>
#endif 
#include <string>
#include <assert.h>
#include <memory>
#include <math.h>
#include "Order.h"
#include "Asset.h"
#include "Exchange.h"
#include "utils_time.h"

void __Exchange::register_asset(__Asset new_asset) {
	new_asset.exchange_id = this->exchange_id;
	this->market.insert({ new_asset.asset_id, new_asset });
	this->asset_counter++;
}
void __Exchange::remove_asset(unsigned int asset_id) {
	this->market.erase(asset_id);
}
void __Exchange::reset() {
	if (this->market.size() == 0) { this->market.swap(this->market_expired); }
	for (auto& kvp : this->market) {
		kvp.second.reset();
	}
	this->asset_counter = this->market.size();
	this->current_index = 0;
	this->orders.clear();
}
void __Exchange::build() {

	for (auto& kvp : this->market) {
		if(kvp.second.slippage > 0){
			this->has_slippage = true;
			break;
		}
	}

	while (true) {
		timeval next_time = MAX_TIME;
		bool update = false;
		if (this->market.size() == 0) { break; }
		//Get the next time available across all assets. This will be the next market time
		for (auto& kvp : this->market) {
			if (kvp.second.current_index < (kvp.second.AM.N)) {
				timeval asset_time = kvp.second.asset_time();
				if (asset_time < next_time) {
					next_time = asset_time;
					update = true;
				}
			}
		}
		for (auto& kvp : this->market) {
			if (kvp.second.current_index >= (kvp.second.AM.N)) {
				continue; 
			}
			if (kvp.second.asset_time() == next_time) {
				kvp.second.current_index++;
			}
		}
		if (!update){
			break;
		};
		this->datetime_index.push_back(next_time);
		long _time = next_time.tv_sec + next_time.tv_usec / 1e6;
		this->epoch_index.push_back(_time);
	}
	for (auto& kvp : this->market) { kvp.second.current_index = 0; }
}
bool __Exchange::_get_market_view() {
	//if the current index is the last then return false, all assets listed on this exchange 
	//are done streaming their data
	if (this->current_index == this->datetime_index.size()) {
		return false;
	}

	//loop through all of the assets in this exchage. If the asset's next datetime is equal to the 
	//next datetime of the exchange, then that asset will be visable in the market view.
	timeval next_time = this->datetime_index[this->current_index];
	for (auto& _asset_pair : this->market) {
		__Asset * const _asset = &_asset_pair.second;

		//if asset time is equal to the next time add pointer to the asset into the market view
		if (_asset->asset_time() == next_time){
			//make sure asset index is greater than it's warmup period
			if(_asset->current_index >= _asset->minimum_warmup){
				this->market_view[_asset->asset_id] = _asset;
			}
			_asset->current_index++;
		}
		//else make sure the market view does not contain a pointer to the asset
		else {
			this->market_view.erase(_asset->asset_id);
		}
		//if the asset's current index is equal to it's last row of data, add the asset's id to the asset
		//remove vector so we can remove it when clean up market is called
		if (_asset->current_index == (_asset->AM.N)+1) {
			this->asset_remove.push_back(_asset->asset_id);
		}
	}
	this->current_time = next_time;
	this->current_index++;
	return true;
}
void __Exchange::clean_up_market(std::vector<std::unique_ptr<Order>> & canceled_orders) {
	if (this->asset_remove.empty()) {
		return;
	}
	for (auto asset_name : this->asset_remove) {
		//delete the asset from the market 
		this->market_expired[asset_name] = std::move(this->market.at(asset_name));
		this->cancel_orders(canceled_orders, asset_name);
		this->market.erase(asset_name);
		this->market_view.erase(asset_name);
	}
	this->asset_remove.clear();
	this->asset_counter--;
}

void __Exchange::_set_slippage(double _slippage){
	for(auto& pair : this->market){
		pair.second.slippage = _slippage;
	}
	this->has_slippage = true;
}

double __Exchange::apply_slippage(unsigned int asset_id, double market_price, double units){
	double asset_slippage = this->market[asset_id].slippage;
	if(asset_slippage == 0){
		return market_price;
	}
	double fill_price;
	if(units > 0){
		fill_price = market_price * (1 + asset_slippage);
		this->total_slippage += ((fill_price - market_price) * units);
		return fill_price;
	}
	else{
		fill_price = market_price * (1 - asset_slippage);
		this->total_slippage += ((fill_price - market_price) * units);
		return fill_price;
	}
}

void __Exchange::process_market_order(MarketOrder * const open_order) {
	double market_price = _get_market_price(open_order->asset_id, open_order->units, open_order->cheat_on_close);
	if (isnan(market_price)) { 
		throw std::invalid_argument("recieved order for which asset has no market price");
	}
	if(this->has_slippage){
		market_price = this->apply_slippage(open_order->asset_id,market_price,open_order->units);
	}
	open_order->fill(market_price, this->current_time);
}

void __Exchange::process_limit_order(LimitOrder *const open_order, bool on_close) {
	double market_price = _get_market_price(open_order->asset_id, open_order->units, on_close);
	if (isnan(market_price)) {
		throw std::invalid_argument("recieved order for which asset has no market price");
	}
	if ((open_order->units > 0) & (market_price <= open_order->limit)) {
		if(this->has_slippage){market_price = this->apply_slippage(open_order->asset_id,market_price,open_order->units);}
		open_order->fill(market_price, this->current_time);
	}
	else if ((open_order->units < 0) & (market_price >= open_order->limit)) {
		if(this->has_slippage){market_price = this->apply_slippage(open_order->asset_id,market_price,open_order->units);}
		open_order->fill(market_price, this->current_time);
	}
}

void __Exchange::process_stoploss_order(StopLossOrder *const open_order, bool on_close){
	double market_price = _get_market_price(open_order->asset_id, open_order->units, on_close);
	if ((open_order->units < 0) & (market_price <= open_order->stop_loss)) {
		if(this->has_slippage){market_price = this->apply_slippage(open_order->asset_id,market_price,open_order->units);}
		open_order->fill(market_price, this->current_time);
		return;
	}
	else if ((open_order->units > 0) & (market_price >= open_order->stop_loss)) {
		if(this->has_slippage){market_price = this->apply_slippage(open_order->asset_id,market_price,open_order->units);}
		open_order->fill(market_price, this->current_time);
		return;
	}
}

void __Exchange::process_order(std::unique_ptr<Order> &open_order, bool on_close) {
	try {
		switch (open_order->order_type) {
			case MARKET_ORDER:
			{
				MarketOrder* order_market = static_cast <MarketOrder*>(open_order.get());
				this->process_market_order(order_market);
				break;
			}
			case LIMIT_ORDER: {
				LimitOrder* order_limit = static_cast <LimitOrder*>(open_order.get());
				this->process_limit_order(order_limit, on_close);
				break;
			}
			case STOP_LOSS_ORDER: {
				StopLossOrder* order_stoploss = static_cast <StopLossOrder*>(open_order.get());
				this->process_stoploss_order(order_stoploss, on_close);
				break;
			}
			case TAKE_PROFIT_ORDER: {
				break;
			}
		}
	}
	catch (const std::exception& e) {
		throw e;
	}
}

std::vector<std::unique_ptr<Order>> __Exchange::process_orders(bool on_close) {
	std::vector<std::unique_ptr<Order>> orders_filled;
	std::deque<std::unique_ptr<Order>> orders_open;

	while (!this->orders.empty()) {
		std::unique_ptr<Order>& order = this->orders[0];
		/*
		Orders are executed at the open and close of every time step. Order member alive is
		false the first time then true after. But the first time an order is evaulated use cheat on close
		to determine wether we can process the order.
		*/
		if (order->cheat_on_close == on_close || order->alive) {
			try {
				this->process_order(order, on_close);
			}
			catch (const std::exception& e) {
				if(this->logging){
					std::cerr << "INVALID ORDER CAUGHT: " << e.what() << std::endl;
				}
			}
		}
		else {
			order->order_create_time = this->current_time;
			order->alive = true;
		}
		//order was not filled so add to open order container
		if (order->order_state != FILLED) {
			orders_open.push_back(std::move(order));
		}
		//order was filled
		else {
			if (this->logging) { this->log_order_filled(order); }
			//if the order has child orders push them into the open order container
			if (order->orders_on_fill.size() > 0) {
				for (auto& child_order : order->orders_on_fill) {
					orders_open.push_back(std::move(child_order));
				}
			}
			//push filled order into filled orders container to send fill info back to broker
			orders_filled.push_back(std::move(order));
		}
		this->orders.pop_front();
	}
	this->orders = std::move(orders_open);
	return orders_filled;
}

bool __Exchange::place_order(std::unique_ptr<Order> new_order) {
	if (this->logging) { (this->log_order_placed(new_order)); }
	this->orders.push_back(std::move(new_order));
	return true;
}

std::unique_ptr<Order> __Exchange::cancel_order(std::unique_ptr<Order>& order_cancel) {
	std::unique_ptr<Order> order;
	for (size_t i = 0; i < this->orders.size(); i++) {
		if ((*this->orders[i]) == *order_cancel) {
			order = std::move(*(this->orders.begin() + i));
			this->orders.erase(this->orders.begin() + i);
			return order;
		}
	}
	return order;
}

void __Exchange::cancel_orders(std::vector<std::unique_ptr<Order>>& canceled_orders, unsigned int asset_id) {
	for (auto& order : this->orders) {
		if (order->asset_id != asset_id) { continue; }
		canceled_orders.push_back(this->cancel_order(order));
	}
}

void __Exchange::log_order_placed(std::unique_ptr<Order>& order) {
	memset(this->time, 0, sizeof this->time);
	timeval_to_char_array(&order->order_create_time, this->time, sizeof(this->time));
	printf("EXCHANGE %s: %s PLACED: strategy_id: %i, exchange_id: %i, asset_id: %i, units: %f\n",
		this->time,
		order->get_order_type(),
		order->strategy_id,
		order->exchange_id,
		order->asset_id,
		order->units
	);
}

void __Exchange::log_order_filled(std::unique_ptr<Order>& order) {
	memset(this->time, 0, sizeof this->time);
	timeval_to_char_array(&order->order_fill_time, this->time, sizeof(this->time));
	printf("EXCHANGE %s: %s FILLED: strategy_id: %i, exchange_id: %i, asset_id: %i, units: %f, fill_price: %f\n",
		this->time,
		order->get_order_type(),
		order->strategy_id,
		order->exchange_id,
		order->asset_id,
		order->units,
		order->fill_price
	);
}

void * CreateExchangePtr(bool logging){
	__Exchange *new_exchange = new __Exchange;
	new_exchange->logging = logging;
	return new_exchange;
}
void DeleteExchangePtr(void *ptr){
	__Exchange * __exchange_ref = static_cast<__Exchange *>(ptr);
	delete __exchange_ref;
}
void reset_exchange(void *exchange_ptr) {
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	__exchange_ref->reset();
}
bool _is_registered(void *exchange_ptr) {
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	return __exchange_ref->is_registered;
}
void set_slippage(void *exchange_ptr, double slippage) {
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	__exchange_ref->_set_slippage(slippage);
}
void register_asset(void *asset_ptr, void *exchange_ptr) {
	__Asset * __asset_ref = static_cast<__Asset *>(asset_ptr);
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	__exchange_ref->register_asset(*__asset_ref);
}
int asset_count(void *exchange_ptr) {
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	return __exchange_ref->asset_counter;
}
void build_exchange(void *exchange_ptr) {
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	__exchange_ref->build();
}
size_t get_exchange_index_length(void *exchange_ptr){
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	return __exchange_ref->epoch_index.size();
}
long *get_exchange_datetime_index(void *exchange_ptr){
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	return __exchange_ref->epoch_index.data();
}
long get_current_datetime(void *exchange_ptr){
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	return __exchange_ref->current_time.tv_sec + (__exchange_ref->current_time.tv_sec / 1e6);
}
void get_market_view(void *exchange_ptr) {
	__Exchange *__exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	__exchange_ref->_get_market_view();
}
double get_market_price(void *exchange_ptr, unsigned int asset_id, double units, bool on_close) {
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	double price =  __exchange_ref->_get_market_price(asset_id,units,on_close);
	return price;
}
void* get_asset_ptr(void *exchange_ptr, unsigned int asset_id) {
	__Exchange * __exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	return &__exchange_ref->market[asset_id];
}
double get_market_feature(void *exchange_ptr, unsigned int asset_id, const char *column, int index) {
	__Exchange *__exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	std::string column_str(column);
	return __exchange_ref->_get_market_feature(asset_id,column_str, index);
}
void get_id_max_market_feature(void *exchange_ptr, const char *column, unsigned int *res_ids, unsigned int count, bool max){
	__Exchange *__exchange_ref = static_cast<__Exchange *>(exchange_ptr);
	std::string column_str(column);
	std::map<unsigned int, double> id_feature_map;

	for(auto const & pair : __exchange_ref->market_view){
		unsigned int asset_id = pair.first;
		id_feature_map[asset_id] = __exchange_ref->_get_market_feature(asset_id,column_str, 0);
	}
	std::vector<unsigned int> ids = n_biggest_elements(id_feature_map, count);

	for(unsigned int i = 0; i < count; i++){
		res_ids[i] = ids[i]; 
	}
}


