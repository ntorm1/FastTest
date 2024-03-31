#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_ast_enums.hpp"
#include "ft_buffer_node.hpp"
#include "standard/ft_linalg.hpp"
#include "ft_observer_base.hpp"
#include "standard/ft_types.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
class SumObserverNode : public ObserverNode {
public:
  SumObserverNode(SharedPtr<BufferOpNode> parent, size_t window,
                  Option<String> name = std::nullopt) noexcept;
  ~SumObserverNode() noexcept;

  void onOutOfRange(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept override;
  void cacheObserver() noexcept override;
  void reset() noexcept override;
};

END_AST_NAMESPACE