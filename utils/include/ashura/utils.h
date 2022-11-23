#pragma once

#include <type_traits>

#include "spdlog/spdlog.h"
#include "stx/enum.h"
#include "stx/limits.h"
#include "stx/option.h"
#include "stx/panic.h"
#include "stx/struct.h"

// TODO(lamarrr): separate into log utils, error utils, iter utils, and enum
// utils, object utils

#define ASR_PANIC(...) ::stx::panic(__VA_ARGS__)

#define ASR_ENSURE(expr, ...)               \
  do {                                      \
    if (!(expr)) ::stx::panic(__VA_ARGS__); \
  } while (false)

#define ASR_LOG(...)             \
  do {                           \
    ::spdlog::info(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_IF(expr, ...)                \
  do {                                       \
    if ((expr)) ::spdlog::info(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_WARN(...)        \
  do {                           \
    ::spdlog::warn(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_ERR(...)          \
  do {                            \
    ::spdlog::error(__VA_ARGS__); \
  } while (false)

#define ASR_LOG_WARN_IF(expr, ...)           \
  do {                                       \
    if ((expr)) ::spdlog::warn(__VA_ARGS__); \
  } while (false)

#define ASR_ERRNUM_CASE(x) \
  case x:                  \
    return #x;

#define ASR_UNREACHABLE() \
  ASR_PANIC("Expected program execution to not reach this state")

#define AS_U32(...) static_cast<u32>(__VA_ARGS__)

#define AS_U64(...) static_cast<u64>(__VA_ARGS__)

#define AS_I64(...) static_cast<i64>(__VA_ARGS__)

#define AS_F32(...) static_cast<f32>(__VA_ARGS__)

namespace asr {

template <typename Container>
constexpr bool any_true(Container const &cont) {
  return ::std::any_of(cont.begin(), cont.end(),
                       [](auto const &value) -> bool { return value; });
}

template <typename Container, typename Value>
constexpr bool any_eq(Container const &cont, Value &&value) {
  return ::std::find(cont.begin(), cont.end(), static_cast<Value &&>(value)) !=
         cont.end();
}

template <typename Container, typename UnaryPredicate>
constexpr bool any(Container const &cont, UnaryPredicate &&predicate) {
  return ::std::any_of(cont.begin(), cont.end(),
                       static_cast<UnaryPredicate &&>(predicate));
}

template <typename Target, typename Source>
STX_FORCE_INLINE stx::Option<Target *> upcast(Source &source) {
  Target *ptr = dynamic_cast<Target *>(&source);
  if (ptr == nullptr) {
    return stx::None;
  } else {
    return stx::Some(static_cast<Target *>(ptr));
  }
}

}  // namespace asr