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
print(EXCHANGE_PATH)

ASSET1_ID = "asset1"
ASSET2_ID = "asset2"

DATETIME_FORMAT = "%Y-%m-%d"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"
