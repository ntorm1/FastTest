#include "ft_types.hpp"
#include "pch.hpp"

static const std::string EXCHANGE_NAME = "test_exchange";
static const std::string EXCHANGE_PATH =
    "C:/Users/natha/OneDrive/Desktop/C++/FastTest/FastTests/exchange1";
static const std::string DATETIME_FORMAT = "%Y-%m-%d";

// Define the asset names
static const std::string ASSET1_NAME = "asset1";
static const std::string ASSET2_NAME = "asset2";

// Define arrays
static const std::vector<double> ASSET1_CLOSE = {101.0, 103.0, 105.0, 106.0};
static const std::vector<double> ASSET2_CLOSE = {101.5, 99.0,  97.0,
                                                 101.5, 101.5, 96.0};