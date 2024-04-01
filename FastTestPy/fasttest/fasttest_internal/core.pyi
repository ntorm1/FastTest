from __future__ import annotations
import fasttest_internal.strategy
import numpy
import typing
__all__ = ['BUILT', 'Exchange', 'FINISHED', 'FTManager', 'FTManagerState', 'FastTestException', 'INIT', 'RUNNING']
class Exchange:
    def getAssetIndex(self, arg0: str) -> int | None:
        ...
    def getAssetMap(self) -> dict[str, int]:
        ...
    def getColumns(self) -> list[str]:
        ...
    def getCurrentIdx(self) -> int:
        ...
    def getData(self) -> numpy.ndarray[numpy.float64[m, n]]:
        ...
    def getTimestamps(self) -> list[int]:
        ...
class FTManager:
    def __init__(self) -> None:
        """
        Initialize the FTManager instance.
        """
    def addExchange(self, name: str, source: str, datetime_format: str | None = None) -> Exchange | None:
        """
        Add an exchange to the FTManager instance.
        
        :param str name: The name of the exchange.
        :param str source: The source of the exchange data.
        :param str datetime_format: The datetime format of the exchange data. Defaults to None.
        """
    def addStrategy(self, strategy: fasttest_internal.strategy.MetaStrategy, replace_if_exsists: bool) -> fasttest_internal.strategy.MetaStrategy | None:
        ...
    def build(self) -> bool:
        ...
    def getExceptions(self, arg0: bool) -> list[FastTestException]:
        ...
    def getExchange(self, name: str) -> Exchange | None:
        """
        Get the exchange by its name.
        
        :param str name: The name of the exchange to retrieve.
        """
    def getGlobalTime(self) -> int:
        """
        Get the global time from the FTManager instance.
        """
    def getState(self) -> FTManagerState:
        ...
    def reset(self) -> None:
        """
        Reset the FTManager instance, reverting it to its initial state.
        """
    def run(self) -> bool:
        ...
    def step(self) -> None:
        """
        Proceed to the next time step in the FTManager instance.
        """
class FTManagerState:
    """
    Members:
    
      INIT
    
      BUILT
    
      RUNNING
    
      FINISHED
    """
    BUILT: typing.ClassVar[FTManagerState]  # value = <FTManagerState.BUILT: 1>
    FINISHED: typing.ClassVar[FTManagerState]  # value = <FTManagerState.FINISHED: 3>
    INIT: typing.ClassVar[FTManagerState]  # value = <FTManagerState.INIT: 0>
    RUNNING: typing.ClassVar[FTManagerState]  # value = <FTManagerState.RUNNING: 2>
    __members__: typing.ClassVar[dict[str, FTManagerState]]  # value = {'INIT': <FTManagerState.INIT: 0>, 'BUILT': <FTManagerState.BUILT: 1>, 'RUNNING': <FTManagerState.RUNNING: 2>, 'FINISHED': <FTManagerState.FINISHED: 3>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class FastTestException:
    def __init__(self, arg0: str) -> None:
        ...
    @property
    def message(self) -> str:
        ...
BUILT: FTManagerState  # value = <FTManagerState.BUILT: 1>
FINISHED: FTManagerState  # value = <FTManagerState.FINISHED: 3>
INIT: FTManagerState  # value = <FTManagerState.INIT: 0>
RUNNING: FTManagerState  # value = <FTManagerState.RUNNING: 2>
