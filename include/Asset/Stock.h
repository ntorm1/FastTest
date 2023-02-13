#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif 
#include "Asset.h"

enum ExchangeType {
    NYSE,
    NADAQ
};

class Stock : public __Asset 
{
public: 
    ExchangeType exchange_type; 
    Stock(unsigned int asset_id,
                        ExchangeType exchange_type,
                        __AssetDataFormat format = __AssetDataFormat(), 
				        unsigned int minimum_warmup = 0, 
				        unsigned int exchange_id = 0)
        :__Asset(asset_id, format, minimum_warmup, exchange_id)
        {
            this->exchange_type = exchange_type;
        }
};

