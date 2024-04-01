#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_ast_enums.hpp"
#include "standard/ft_types.hpp"

BEGIN_AST_NAMESPACE

struct NodeFactoryImpl;

class NodeFactory {
private:
  UniquePtr<NodeFactoryImpl> m_impl;

public:
  FASTTEST_API NodeFactory(NonNullPtr<ASTStrategy> strategy) noexcept;
  FASTTEST_API ~NodeFactory() noexcept;

  FASTTEST_API Option<SharedPtr<ReadOpNode>>
  createReadOpNode(String const &column, int row_offset) noexcept;

  FASTTEST_API Option<SharedPtr<BinOpNode>>
  createBinOpNode(SharedPtr<BufferOpNode> left, BinOpType op,
                  SharedPtr<BufferOpNode> right) noexcept;

  FASTTEST_API Option<SharedPtr<UnaryOpNode>>
  createUnaryOpNode(SharedPtr<BufferOpNode> left, UnaryOpType op,
                    Option<double> op_param = std::nullopt) noexcept;

  FASTTEST_API SharedPtr<ReduceOpNode>
  createReduceOp(SharedPtr<BufferOpNode> node,
                 Vector<std::pair<ReduceOpType, double>> filters) noexcept;

  FASTTEST_API SharedPtr<ObserverNode>
  createSumObserverNode(SharedPtr<BufferOpNode> node, size_t window,
                        Option<String> name = std::nullopt) noexcept;

  FASTTEST_API Option<SharedPtr<AllocationNode>>
  createAllocationNode(SharedPtr<BufferOpNode> parent,
                       AllocationType alloc_type, double epsilon = 0.0f,
                       Option<double> alloc_param = std::nullopt) noexcept;
};

END_AST_NAMESPACE