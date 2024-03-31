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
        self.assertEqual(len(timestamps), 6)

    def testRun(self) -> None:
        self.assertTrue(self.manager.build())
        self.assertTrue(self.manager.run())
        assert True


if __name__ == "__main__":
    unittest.main()
