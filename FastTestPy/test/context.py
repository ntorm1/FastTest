import unittest
import logging
import os
import sys

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import fasttest

EXCHANGE_PATH = os.path.join(os.path.dirname(__file__), "exchange1")
print(EXCHANGE_PATH)


DATETIME_FORMAT = "%Y-%m-%d"
STRATEGY_ID = "test_strategy_1"
EXCHANGE_ID = "test_exchange_1"
