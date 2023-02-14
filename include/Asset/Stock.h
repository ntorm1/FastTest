#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif 
#include "Asset.h"
#include "utils_time.h"

enum ExchangeType {
    NYSE,
    NASDAQ
};

class Stock : public __Asset 
{
public: 
    ExchangeType exchange_type; 

    unsigned int open_market_hour;
    unsigned int open_market_minute;
    unsigned int close_market_hour;
    unsigned int close_market_minute;

    Stock(unsigned int asset_id,ExchangeType exchange_type,
                    __AssetDataFormat format = __AssetDataFormat(), 
				    unsigned int minimum_warmup = 0, 
				    unsigned int exchange_id = 0)
        :__Asset(asset_id, format, minimum_warmup, exchange_id)
        {
            this->exchange_type = exchange_type;
            if(exchange_type == NYSE || exchange_type == NASDAQ){
                this->open_market_hour = UTC_US_EQUITY_HR_OPEN;
                this->open_market_minute = UTC_US_EQUITY_MIN_OPEN;
                this->close_market_hour = UTC_US_EQUITY_HR_CLOSE;
                this->close_market_minute = UTC_US_EQUITY_MIN_CLOSE;
            }
        }
};

