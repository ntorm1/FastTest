import sys
import os
import unittest

import numpy as np

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir)))

from Exchange import Exchange, Asset
from Broker import Broker
from Strategy import *
from FastTest import FastTest
from helpers import *

class TradeTestMethods(unittest.TestCase):
    def test_seperate_trades(self):
        print("TESTING test_seperate_trades...")
        orders = [
                OrderSchedule(
                    order_type = OrderType.MARKET_ORDER,
                    asset_name = "2",
                    i = 1,
                    units = 100
                ),
                OrderSchedule(
                    order_type = OrderType.MARKET_ORDER,
                    asset_name = "2",
                    i = 3,
                    units = -50,
                    trade_id = -1
                )
            ]
        exchange, broker, ft = setup_multi(logging=False)
        strategy = TestStrategy(orders, broker, exchange)
        ft.add_strategy(strategy)
        ft.run()
        
        trade_history = broker.get_trade_history()
        order_history = broker.get_order_history()
        position_history = broker.get_position_history()
        
        assert(len(order_history) == 2)
        assert(len(position_history) == 1)
        assert(len(trade_history) == 2)
        

        assert(trade_history.TRADE_ARRAY[0].contents.average_price == 101)
        assert(trade_history.TRADE_ARRAY[1].contents.average_price == 98)
        assert(trade_history.TRADE_ARRAY[0].contents.close_price == 96)
        assert(trade_history.TRADE_ARRAY[1].contents.close_price == 96)
        assert(trade_history.TRADE_ARRAY[0].contents.realized_pl == 250)
        assert(trade_history.TRADE_ARRAY[1].contents.realized_pl == -200)
        
        assert(position_history.POSITION_ARRAY[0].contents.realized_pl == 50)
        
        assert(broker.get_nlv_history()[-1] == 100050)
        
        print("TESTING: test_seperate_trades passed")
        
    def test_trade_increase(self):
        print("TESTING test_trade_increase...")
        orders = [
                OrderSchedule(
                    order_type = OrderType.MARKET_ORDER,
                    asset_name = "2",
                    i = 1,
                    units = 100
                ),
                OrderSchedule(
                    order_type = OrderType.MARKET_ORDER,
                    asset_name = "2",
                    i = 2,
                    units = 100,
                    trade_id = -1
                )
            ]
        exchange, broker, ft = setup_multi(logging=False)
        strategy = TestStrategy(orders, broker, exchange)
        ft.add_strategy(strategy)
        ft.run()
        
        trade_history = broker.get_trade_history()
        order_history = broker.get_order_history()
        position_history = broker.get_position_history()
        
        assert(trade_history.TRADE_ARRAY[0].contents.realized_pl == -500)
        assert(trade_history.TRADE_ARRAY[1].contents.realized_pl == -200)
        assert(broker.get_nlv_history()[-1] == (100000-700))

        print("TESTING: test_trade_increase passed")
if __name__ == '__main__':
    unittest.main()
