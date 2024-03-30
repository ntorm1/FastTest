#include "pch.hpp"
#include "ft_macros.hpp"
#include "ft_error.hpp"
#include "ft_declare.hpp"
#include <cstdint>
#include <exception>


BEGIN_FASTTEST_NAMESPACE

template <typename T>
using Set = absl::flat_hash_set<T>;

template <typename K, typename V>
using Map = absl::flat_hash_map<K, V>;

template <typename T> using Vector = std::vector<T>;

template <typename T> using UniquePtr = std::unique_ptr<T>;

template <typename T> using SharedPtr = std::shared_ptr<T>;

template <typename T> using NonNullPtr = gsl::not_null<T*>;

template <typename T> using Option = std::optional<T>;

template <typename T, typename E> using Result = tl::expected<T, E>;

template <typename T> using FastTestResult = Result<T, FastTestException>;

template <typename T> using Err = tl::unexpected<T>;

using String = std::string;


using Int = int;
using Int8 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

using Uint = unsigned int;
using Uint8 = uint8_t;
using Uint16 = uint16_t;
using Uint32 = uint32_t;
using Uint64 = uint64_t;



END_FASTTEST_NAMESPACE