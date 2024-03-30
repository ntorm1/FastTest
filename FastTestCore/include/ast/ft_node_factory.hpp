#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_types.hpp"

BEGIN_AST_NAMESPACE

struct NodeFactoryImpl;

class NodeFactory {
private:
  UniquePtr<NodeFactoryImpl> m_impl;

public:
  FASTTEST_API NodeFactory(SharedPtr<Exchange> exchange) noexcept;
  FASTTEST_API ~NodeFactory() noexcept;

  FASTTEST_API Option<SharedPtr<ReadOpNode>>
  createReadOpNode(String const &column, int row_offset) noexcept;
};

END_AST_NAMESPACE