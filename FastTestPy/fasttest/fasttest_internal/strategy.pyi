from __future__ import annotations
import fasttest_internal.ast
import fasttest_internal.core
import numpy
import typing
__all__ = ['ASTStrategy', 'MetaStrategy', 'StrategyAllocator', 'StrategyAllocatorConfig']
class ASTStrategy(StrategyAllocator):
    def __init__(self, name: str, parent: StrategyAllocator, config: StrategyAllocatorConfig) -> None:
        ...
    def getNodeFactory(self) -> fasttest_internal.ast.NodeFactory:
        ...
class MetaStrategy(StrategyAllocator):
    def __init__(self, name: str, exchange: fasttest_internal.core.Exchange, config: StrategyAllocatorConfig, starting_cash: float, parent: StrategyAllocator | None = None) -> None:
        ...
    def addStrategy(self, strategy: StrategyAllocator, replace_if_exsists: bool) -> StrategyAllocator | None:
        ...
class StrategyAllocator:
    def getAllocationBuffer(self) -> numpy.ndarray[numpy.float64[m, 1]]:
        ...
    def getAssetAllocation(self, arg0: int) -> float | None:
        ...
    def getTracer(self) -> ...:
        ...
class StrategyAllocatorConfig:
    allocation: float
    can_short: bool
    disable_on_breach: bool
    risk_contrib_limit: float | None
    vol_limit: float | None
    vol_target: float | None
    weight_clip: float | None
    def __init__(self) -> None:
        ...
