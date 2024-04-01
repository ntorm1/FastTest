import unittest
import logging
import os
import sys
from typing import *

import numpy as np

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import fasttest
from fasttest.util import exchangeToDataFrame


EXCHANGE_PATH = os.path.join(os.path.dirname(__file__), "exchange1")
EXCHANGE_BTC_PATH = os.path.join(os.path.dirname(__file__), "exchange_btc")

ASSET1_ID = "asset1"
ASSET2_ID = "asset2"

DATETIME_FORMAT = "%Y-%m-%d"
DATETIME_FORMAT_BTC = "%Y-%m-%d %H:%M:%S+00:00"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"
EXCHANGE_BTC_ID = "test_exchange_btc"

STARTING_CASH = 1000000.0
