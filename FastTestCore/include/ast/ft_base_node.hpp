#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_types.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
enum class NodeType {
  BIN_OP = 0,
  UNARY_OP = 1,
  ASSET_READ = 2,
  ASSET_OBSERVER = 3,
};

//============================================================================
class ASTNode {

private:
  Vector<NonNullPtr<ASTNode>> m_parent;
  size_t m_warmup;
  NodeType m_type;

protected:
  Exchange &m_exchange;

public:
  ASTNode(Exchange &exchange, NodeType type, size_t warmup,
          Option<NonNullPtr<ASTNode>> parent = std::nullopt) noexcept
      : m_exchange(exchange), m_type(type), m_warmup(warmup) {
    if (parent.has_value()) {
      m_parent.push_back(parent.value());
    }
  }
  ASTNode(Exchange &exchange, NodeType type, size_t warmup,
          Vector<NonNullPtr<ASTNode>> parent) noexcept
      : m_exchange(exchange), m_type(type), m_parent(std::move(parent)),
        m_warmup(warmup) {}
  ASTNode(const ASTNode &) = delete;

  virtual ~ASTNode() {}
  virtual void reset() noexcept = 0;
  void setWarmup(size_t warmup) noexcept { m_warmup = warmup; }
  [[nodiscard]] auto &getExchange() noexcept { return m_exchange; }
  [[nodiscard]] auto &getParentsMut() noexcept { return m_parent; }
  [[nodiscard]] auto const &getParents() const noexcept { return m_parent; }
  NodeType getType() const noexcept { return m_type; }
  [[nodiscard]] FASTTEST_API size_t getWarmup() const noexcept {
    return m_warmup;
  }
};

//============================================================================
template <typename T> class ExpressionNode : public ASTNode {
public:
  template <typename... Args>
  ExpressionNode(Args &&...args) noexcept
      : ASTNode(std::forward<Args>(args)...) {}
  virtual ~ExpressionNode() {}
  virtual T evaluate() noexcept = 0;
};

//============================================================================
template <typename Result, typename... Params>
class OpperationNode : public ASTNode {
public:
  template <typename... Args>
  OpperationNode(Args &&...args) noexcept
      : ASTNode(std::forward<Args>(args)...) {}
  virtual ~OpperationNode() {}
  virtual Result evaluate(Params...) noexcept = 0;
};

//============================================================================
class StatementNode : public ASTNode {
public:
  template <typename... Args>
  StatementNode(Args &&...args) noexcept
      : ASTNode(std::forward<Args>(args)...) {}
  virtual ~StatementNode() {}
  virtual void evaluate() noexcept = 0;
};

END_AST_NAMESPACE