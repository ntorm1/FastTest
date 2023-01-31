#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include "Asset.h"

namespace Risk {

    template <typename T>
    T beta(const std::vector<T> &market_prices, const std::vector<T> &stock_prices) {
        T mean_market_returns = 0.0;
        T sum_market_square_dev = 0.0;
        T sum_stock_market_cov = 0.0;
        T sum_stock_square_dev = 0.0;
        int size = market_prices.size();

        for (int i = 1; i < size; ++i) {
            T market_return = (market_prices[i] - market_prices[i - 1]) / market_prices[i - 1];
            T stock_return = (stock_prices[i] - stock_prices[i - 1]) / stock_prices[i - 1];

            mean_market_returns += market_return;
            sum_market_square_dev += market_return * market_return;
            sum_stock_market_cov += market_return * stock_return;
            sum_stock_square_dev += stock_return * stock_return;
        }

        mean_market_returns /= (size - 1);
        sum_market_square_dev /= (size - 1);
        sum_stock_market_cov /= (size - 1);
        sum_stock_square_dev /= (size - 1);

        T market_variance = sum_market_square_dev - mean_market_returns * mean_market_returns;
        T beta = sum_stock_market_cov / market_variance;

        return beta;
    }
    template <typename T>
    T beta(const __Asset *asset, const __Asset *benchmark, unsigned int n){
        T mean_market_returns = 0.0;
        T sum_market_square_dev = 0.0;
        T sum_stock_market_cov = 0.0;
        T sum_stock_square_dev = 0.0;

        if(n > asset->datetime_index.size()){
            throw std::runtime_error("beta lookback period greater then number of rows");
        }

        unsigned int asset_end_index = asset->current_index - 1;
        unsigned int asset_start_index = asset_start_index - n;
        unsigned int benchmark_end_index = benchmark->current_index - 1;
        unsigned int benchmark_start_index = benchmark_end_index - n;

        unsigned int asset_step_size = asset->AM.col_size();
        unsigned int benchmark_step_size = asset->AM.col_size();
        std::vector<T> &stock_prices = asset->AM.data;
        std::vector<T> &market_prices = benchmark->AM.data;

        unsigned int asset_col = asset->close_col_ask;
        unsigned int benchmark_col = benchmark->close_col_ask;

        unsigned int asset_index = asset_start_index + asset_col;
        unsigned int benchmark_index = benchmark_start_index + benchmark_col;
        for (unsigned int i = 1; i < n; i++) {
            T market_return = (market_prices[benchmark_index+i] - market_prices[benchmark_index+i - 1]) 
                                / market_prices[benchmark_index+i - 1];
            T stock_return = (market_prices[asset_index+i] - market_prices[asset_index+i - 1]) 
                                / market_prices[asset_index+i - 1];

            asset_index += asset_step_size;
            benchmark_index += benchmark_step_size;

            mean_market_returns += market_return;
            sum_market_square_dev += market_return * market_return;
            sum_stock_market_cov += market_return * stock_return;
            sum_stock_square_dev += stock_return * stock_return;
        }

        mean_market_returns /= (n - 1);
        sum_market_square_dev /= (n - 1);
        sum_stock_market_cov /= (n - 1);
        sum_stock_square_dev /= (n - 1);

        T market_variance = sum_market_square_dev - mean_market_returns * mean_market_returns;
        T beta = sum_stock_market_cov / market_variance;

        return beta;
    }

    template <typename T>
    T variance(const std::vector<T> &vec) {
        T mean = 0.0, sum_square_dev = 0.0;
        int size = vec.size();
        for (T &val : vec) {
            mean += val;
            sum_square_dev += val * val;
        }
        mean /= size;
        T variance = (sum_square_dev - size * mean * mean) / (size - 1);
        return variance;
    }
}