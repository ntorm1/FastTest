#include <unordered_map>
#include "Position.h"
#include "Exchange.h"
#include "Broker.h"

void __Account::reset(){
    this->cash = this->starting_cash;
    this->unrealized_pl = 0;
	this->realized_pl = 0;
    this->net_liquidation_value = this->cash;
    this->portfolio.clear();
}

void __Account::build(double _cash){
    this->reset();
    this->cash = _cash;
    this->net_liquidation_value = _cash;
}

void __Account::set_margin(bool margin){
    this->margin = margin;
}

double __Account::_beta_dollars(unsigned int n){
    double _sum = 0;
    for(auto & pair : this->portfolio){
        std::unique_ptr<Position> &position = pair.second;
        _sum += position->beta_dollars(this->beta_benchmark, n);
    }
    return _sum;
}

void __Account::evaluate_account(bool on_close){
    double nlv = 0;
    double collateral = 0;
    double margin_req_mid;
    unsigned int asset_id;
    double market_price;

    __Exchange *exchange;
    for (auto it = this->portfolio.begin(); it != this->portfolio.end();) {
        //update portfolio net liquidation value

        asset_id = it->first;
        std::unique_ptr<Position>& position =  it->second;

        unsigned int exchange_id = position->exchange_id;
        exchange = this->broker->exchanges[exchange_id];
        market_price = exchange->_get_market_price(asset_id, position->units, on_close);


        //if no market price is available at the time then position cannot be evaluated
        if(market_price == NAN){
            continue;
        }

		//check to see if the underlying asset of the position has finished streaming
        //if so we have to close the current position on close of the current step
        if (exchange->market[asset_id].is_last_view() & on_close) {
            if(this->margin){
                if(position->units < 0){
                    margin_req_mid = this->broker->short_margin_req;
                }
                else{
                    margin_req_mid = this->broker->margin_req;
                }
                double new_collateral = abs(margin_req_mid * position->units*market_price);
                double adjustment = (new_collateral - position->collateral);
                
                if(position->units > 0){
                    this->cash += adjustment;
                }
                else{
                    this->cash -= adjustment;
                }
                position->collateral = new_collateral;
            }
            this->broker->close_position(position, market_price, exchange->current_time);
            it = this->portfolio.erase(it);
        }
        else {
            position->evaluate(market_price, on_close);
            nlv += position->liquidation_value();

            if(this->margin){
                double old_collateral = position->collateral;
                this->broker->margin_adjustment(position, market_price);

                //update the margin required to maintain the position. 
                double adjustment = (position->collateral - old_collateral);

                //if long position, add collateral to subtract off later to prevent double counting the
                //value of the security 
                if(position->units > 0){
                    this->cash += adjustment;
                    collateral += position->collateral;
                }
                //if short position, add the collateral back into nlv to maintain balanced counting
                else{
                    this->cash -= adjustment;
                    nlv += position->collateral;
                }
            }
            it++;
        }
    }
    this->net_liquidation_value = nlv + this->cash - collateral;
}

void beta_hedge(__Account * account, unsigned int n){
    //map between position pointers and units of the hedge allocated to that position
    std::unordered_map<Position*, double, PositionHash> allocations;

    double net_beta_dollars = 0;
    for(auto & position_pair : account->portfolio){
        std::unique_ptr<Position> &position = position_pair.second;
        double beta_dollars = position->beta_dollars(account->beta_benchmark, n);
        net_beta_dollars += beta_dollars;
        allocations[position.get()] = beta_dollars;
    }

    //get the current market price of the benchmark
    unsigned int benchmark_id = account->beta_benchmark->asset_id;
    __Exchange *benchmark_exchange = account->broker->exchanges[benchmark_id];

    double benchmark_market_price; double benchmark_units = 0.0;
    double benchmark_direction = net_beta_dollars * -1;
    benchmark_market_price = benchmark_exchange->_get_market_price(benchmark_id,benchmark_direction,true);

    //set the allocations of the hedge to the positions in the portfolio
    for(auto & position_pair : account->portfolio){
        std::unique_ptr<Position> &position = position_pair.second;
        double beta_dollars = allocations[position.get()];
        double units = beta_dollars / benchmark_market_price;
        allocations[position.get()] = units;
        benchmark_units += units;
    }

    if(abs(benchmark_units * benchmark_market_price) < BETA_REBALANCE_THRESHOLD){
        return;
    }

    //create new market order to for the hedge position
    std::unique_ptr<Order> order(new MarketOrder(
		benchmark_id,
		benchmark_units,
		true,
		benchmark_exchange->exchange_id,
		account->account_id
	));

    //process the order on the exchange
    std::unique_ptr<Order> &order_ref = order;
    benchmark_exchange->process_order(order_ref, true);

    //process the order on the broker side
    std::vector<std::unique_ptr<Order>> order_container;
    order_container.push_back(std::move(order));
    account->broker->process_filled_orders(std::move(order_container));

    //add pointer to the hedge for all positions
    for(auto & position_pair : account->portfolio){
        std::unique_ptr<Position> &position = position_pair.second;
        position->child_positions.push_back(&account->portfolio[benchmark_id]);
    }
}

void * CreateAccountPtr(unsigned int account_id, double cash, const __Asset* benchmark) {
	return new __Account(account_id, cash, benchmark);
}
void DeleteAccountPtr(void *ptr) {
	__Account * __account_ref = static_cast<__Account *>(ptr);
	delete __account_ref;
}
void * GetAccountPtr(void * broker_ptr, unsigned int account_id) {
    __Broker * __broker_ref = static_cast<__Broker *>(broker_ptr);
    return &__broker_ref->accounts[account_id];
}
size_t account_get_history_length(void *account_ptr){
	__Account * __account_ref = static_cast<__Account *>(account_ptr);
	return __account_ref->nlv_history.size();
}
double * account_get_nlv_history(void *account_ptr) {
	__Account * __account_ref = static_cast<__Account *>(account_ptr);
	return __account_ref->nlv_history.data();
}
double * account_get_cash_history(void *account_ptr) {
	__Account * __account_ref = static_cast<__Account *>(account_ptr);
	return __account_ref->cash_history.data();
}
double get_beta_dollars(void *account_ptr, unsigned int lookback){
    __Account * __account_ref = static_cast<__Account *>(account_ptr);
    return __account_ref->_beta_dollars(lookback);
}