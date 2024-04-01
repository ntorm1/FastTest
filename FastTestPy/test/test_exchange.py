from context import *

from fasttest import fasttest_internal


class SimpleExchangeTest(unittest.TestCase):
    def setUp(self) -> NoReturn:
        self.manager = fasttest.fasttest_internal.core.FTManager()
        self.exchange = self.manager.addExchange(
            EXCHANGE_ID, EXCHANGE_PATH, DATETIME_FORMAT
        )
        self.manager.build()
        self.asset_id_1 = self.exchange.getAssetIndex(ASSET1_ID)
        self.asset_id_2 = self.exchange.getAssetIndex(ASSET2_ID)
        config = fasttest_internal.strategy.StrategyAllocatorConfig()
        self.root = fasttest_internal.strategy.MetaStrategy(
            "root",
            exchange=self.exchange,
            config=config,
            starting_cash=STARTING_CASH,
            parent=None,
        )
        assert self.asset_id_1 is not None
        assert self.asset_id_2 is not None
        pass

    def testBuild(self) -> NoReturn:
        timestamps = self.exchange.getTimestamps()
        self.assertEqual(len(timestamps), 6)

    def testRun(self) -> NoReturn:
        self.assertTrue(self.manager.build())
        self.assertTrue(self.manager.run())
        assert True

    def testColumns(self) -> NoReturn:
        columns = self.exchange.getColumns()
        self.assertEqual(columns, ["open", "close"])
        assert True

    def testSumObserver(self) -> NoReturn:
        window = 1
        factory = fasttest_internal.ast.NodeFactory(
            exchange=self.exchange, strategy=self.root
        )
        close = factory.createReadOpNode("close")
        sum_op = factory.createSumObserverNode(close, 2)
        sum_op.enableCache(True)
        self.manager.run()
        sum_cache = sum_op.getCache()
        df = exchangeToDataFrame(self.exchange)
        for asset_id in [ASSET1_ID, ASSET2_ID]:
            asset_index = self.exchange.getAssetIndex(asset_id)
            self.assertTrue(
                np.allclose(
                    sum_cache[asset_index, window:],
                    df[asset_id]["close"]
                    .rolling(2, min_periods=0)
                    .sum()
                    .values[window:],
                )
            )


class BtcExchangeTests(unittest.TestCase):
    def setUp(self) -> NoReturn:
        self.manager = fasttest.fasttest_internal.core.FTManager()
        self.exchange = self.manager.addExchange(
            EXCHANGE_BTC_ID, EXCHANGE_BTC_PATH, DATETIME_FORMAT_BTC
        )
        self.manager.build()
        self.btc_id = self.exchange.getAssetIndex("BTC-USD")
        assert self.btc_id is not None
        pass

    def testBuild(self) -> NoReturn:
        pass


if __name__ == "__main__":
    unittest.main()
