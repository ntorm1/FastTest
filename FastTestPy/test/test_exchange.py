from context import *


class SimpleExchangeTest(unittest.TestCase):
    def setUp(self) -> None:
        self.manager = fasttest.fasttest_internal.core.FTManager()
        self.exchange = self.manager.addExchange(
            EXCHANGE_ID, EXCHANGE_PATH, DATETIME_FORMAT
        )
        pass

    def testBuild(self) -> None:
        timestamps = self.exchange.getTimestamps()
        assert len(timestamps) == 6


if __name__ == "__main__":
    unittest.main()
