from __future__ import annotations
import numpy
import typing
__all__ = ['AllocationType', 'BinOpNode', 'BinOpType', 'BufferOpNode', 'NodeFactory', 'NodeType', 'ObserverNode', 'ObserverType', 'ReadOpNode', 'ReduceOpNode', 'ReduceOpType', 'SumObserverNode', 'UnaryOpNode', 'UnaryOpType']
class AllocationType:
    """
    Members:
    
      UNIFORM
    
      CONDITIONAL_SPLIT
    
      FIXED
    
      NLARGEST
    
      NSMALLEST
    
      NEXTREME
    
      INPLACE
    """
    CONDITIONAL_SPLIT: typing.ClassVar[AllocationType]  # value = <AllocationType.CONDITIONAL_SPLIT: 1>
    FIXED: typing.ClassVar[AllocationType]  # value = <AllocationType.FIXED: 2>
    INPLACE: typing.ClassVar[AllocationType]  # value = <AllocationType.INPLACE: 6>
    NEXTREME: typing.ClassVar[AllocationType]  # value = <AllocationType.NEXTREME: 5>
    NLARGEST: typing.ClassVar[AllocationType]  # value = <AllocationType.NLARGEST: 3>
    NSMALLEST: typing.ClassVar[AllocationType]  # value = <AllocationType.NSMALLEST: 4>
    UNIFORM: typing.ClassVar[AllocationType]  # value = <AllocationType.UNIFORM: 0>
    __members__: typing.ClassVar[dict[str, AllocationType]]  # value = {'UNIFORM': <AllocationType.UNIFORM: 0>, 'CONDITIONAL_SPLIT': <AllocationType.CONDITIONAL_SPLIT: 1>, 'FIXED': <AllocationType.FIXED: 2>, 'NLARGEST': <AllocationType.NLARGEST: 3>, 'NSMALLEST': <AllocationType.NSMALLEST: 4>, 'NEXTREME': <AllocationType.NEXTREME: 5>, 'INPLACE': <AllocationType.INPLACE: 6>}
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
class BinOpNode(BufferOpNode):
    pass
class BinOpType:
    """
    Members:
    
      ADD
    
      SUB
    
      MUL
    
      DIV
    """
    ADD: typing.ClassVar[BinOpType]  # value = <BinOpType.ADD: 0>
    DIV: typing.ClassVar[BinOpType]  # value = <BinOpType.DIV: 3>
    MUL: typing.ClassVar[BinOpType]  # value = <BinOpType.MUL: 2>
    SUB: typing.ClassVar[BinOpType]  # value = <BinOpType.SUB: 1>
    __members__: typing.ClassVar[dict[str, BinOpType]]  # value = {'ADD': <BinOpType.ADD: 0>, 'SUB': <BinOpType.SUB: 1>, 'MUL': <BinOpType.MUL: 2>, 'DIV': <BinOpType.DIV: 3>}
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
class BufferOpNode:
    def address(self) -> int:
        ...
    def enableCache(self, arg0: bool) -> None:
        ...
    def getCache(self) -> numpy.ndarray[numpy.float64[m, n]]:
        ...
    def hasCache(self) -> bool:
        ...
class NodeFactory:
    def createAllocationNode(self, parent: BufferOpNode, alloc_type: AllocationType, epsilon: float = 0.0, alloc_param: float | None = None) -> ... | None:
        ...
    def createBinOpNode(self, left: BufferOpNode, op: BinOpType, right: BufferOpNode) -> BinOpNode | None:
        ...
    def createReadOpNode(self, column: str, row_offset: int = 0) -> ReadOpNode | None:
        ...
    def createSumObserverNode(self, node: BufferOpNode, window: int, name: str | None = None) -> ObserverNode:
        ...
    def createUnaryOpNode(self, parent: BufferOpNode, op: UnaryOpType, op_param: float | None) -> UnaryOpNode | None:
        ...
class NodeType:
    """
    Members:
    
      BIN_OP
    
      UNARY_OP
    
      ASSET_READ
    
      ASSET_OBSERVER
    
      REDUCE_OP
    
      ALLOCATION
    """
    ALLOCATION: typing.ClassVar[NodeType]  # value = <NodeType.ALLOCATION: 5>
    ASSET_OBSERVER: typing.ClassVar[NodeType]  # value = <NodeType.ASSET_OBSERVER: 3>
    ASSET_READ: typing.ClassVar[NodeType]  # value = <NodeType.ASSET_READ: 2>
    BIN_OP: typing.ClassVar[NodeType]  # value = <NodeType.BIN_OP: 0>
    REDUCE_OP: typing.ClassVar[NodeType]  # value = <NodeType.REDUCE_OP: 4>
    UNARY_OP: typing.ClassVar[NodeType]  # value = <NodeType.UNARY_OP: 1>
    __members__: typing.ClassVar[dict[str, NodeType]]  # value = {'BIN_OP': <NodeType.BIN_OP: 0>, 'UNARY_OP': <NodeType.UNARY_OP: 1>, 'ASSET_READ': <NodeType.ASSET_READ: 2>, 'ASSET_OBSERVER': <NodeType.ASSET_OBSERVER: 3>, 'REDUCE_OP': <NodeType.REDUCE_OP: 4>, 'ALLOCATION': <NodeType.ALLOCATION: 5>}
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
class ObserverNode(BufferOpNode):
    pass
class ObserverType:
    """
    Members:
    
      SUM
    
      MEAN
    
      ATR
    
      MAX
    
      TS_ARGMAX
    
      VARIANCE
    
      COVARIANCE
    
      CORRELATION
    
      LINEAR_DECAY
    
      SKEWNESS
    """
    ATR: typing.ClassVar[ObserverType]  # value = <ObserverType.ATR: 2>
    CORRELATION: typing.ClassVar[ObserverType]  # value = <ObserverType.CORRELATION: 7>
    COVARIANCE: typing.ClassVar[ObserverType]  # value = <ObserverType.COVARIANCE: 6>
    LINEAR_DECAY: typing.ClassVar[ObserverType]  # value = <ObserverType.LINEAR_DECAY: 8>
    MAX: typing.ClassVar[ObserverType]  # value = <ObserverType.MAX: 3>
    MEAN: typing.ClassVar[ObserverType]  # value = <ObserverType.MEAN: 1>
    SKEWNESS: typing.ClassVar[ObserverType]  # value = <ObserverType.SKEWNESS: 9>
    SUM: typing.ClassVar[ObserverType]  # value = <ObserverType.SUM: 0>
    TS_ARGMAX: typing.ClassVar[ObserverType]  # value = <ObserverType.TS_ARGMAX: 4>
    VARIANCE: typing.ClassVar[ObserverType]  # value = <ObserverType.VARIANCE: 5>
    __members__: typing.ClassVar[dict[str, ObserverType]]  # value = {'SUM': <ObserverType.SUM: 0>, 'MEAN': <ObserverType.MEAN: 1>, 'ATR': <ObserverType.ATR: 2>, 'MAX': <ObserverType.MAX: 3>, 'TS_ARGMAX': <ObserverType.TS_ARGMAX: 4>, 'VARIANCE': <ObserverType.VARIANCE: 5>, 'COVARIANCE': <ObserverType.COVARIANCE: 6>, 'CORRELATION': <ObserverType.CORRELATION: 7>, 'LINEAR_DECAY': <ObserverType.LINEAR_DECAY: 8>, 'SKEWNESS': <ObserverType.SKEWNESS: 9>}
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
class ReadOpNode(BufferOpNode):
    pass
class ReduceOpNode(BufferOpNode):
    pass
class ReduceOpType:
    """
    Members:
    
      GREATER_THAN
    
      LESS_THAN
    
      EQUAL
    """
    EQUAL: typing.ClassVar[ReduceOpType]  # value = <ReduceOpType.EQUAL: 2>
    GREATER_THAN: typing.ClassVar[ReduceOpType]  # value = <ReduceOpType.GREATER_THAN: 0>
    LESS_THAN: typing.ClassVar[ReduceOpType]  # value = <ReduceOpType.LESS_THAN: 1>
    __members__: typing.ClassVar[dict[str, ReduceOpType]]  # value = {'GREATER_THAN': <ReduceOpType.GREATER_THAN: 0>, 'LESS_THAN': <ReduceOpType.LESS_THAN: 1>, 'EQUAL': <ReduceOpType.EQUAL: 2>}
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
class SumObserverNode(ObserverNode):
    pass
class UnaryOpNode(BufferOpNode):
    pass
class UnaryOpType:
    """
    Members:
    
      SCALAR
    
      SIGN
    
      POWER
    
      ABS
    
      LOG
    """
    ABS: typing.ClassVar[UnaryOpType]  # value = <UnaryOpType.ABS: 3>
    LOG: typing.ClassVar[UnaryOpType]  # value = <UnaryOpType.LOG: 4>
    POWER: typing.ClassVar[UnaryOpType]  # value = <UnaryOpType.POWER: 2>
    SCALAR: typing.ClassVar[UnaryOpType]  # value = <UnaryOpType.SCALAR: 0>
    SIGN: typing.ClassVar[UnaryOpType]  # value = <UnaryOpType.SIGN: 1>
    __members__: typing.ClassVar[dict[str, UnaryOpType]]  # value = {'SCALAR': <UnaryOpType.SCALAR: 0>, 'SIGN': <UnaryOpType.SIGN: 1>, 'POWER': <UnaryOpType.POWER: 2>, 'ABS': <UnaryOpType.ABS: 3>, 'LOG': <UnaryOpType.LOG: 4>}
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
