/// SPDX-License-Identifier: MIT
#pragma once
#include "ashura/std/error.h"
#include "ashura/std/log.h"

namespace ash
{

template <typename T = Void>
struct [[nodiscard]] Ok
{
  using Type = T;
  T value{};
};

template <typename T>
Ok(T) -> Ok<T>;

template <typename E = Void>
struct [[nodiscard]] Err
{
  using Type = E;
  E value{};
};

template <typename T>
Err(T) -> Err<T>;

template <typename T = Void, typename E = Void>
struct [[nodiscard]] Result
{
  using Type    = T;
  using ErrType = E;

  usize is_ok_;

  union
  {
    T value_;
    E err_;
  };

  constexpr Result() = delete;

  constexpr ~Result()
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      err_.~E();
    }
  }

  constexpr Result(Ok<T> ok) : is_ok_{true}, value_{static_cast<T &&>(ok.value)}
  {
  }

  constexpr Result(Err<E> err) :
      is_ok_{false},
      err_{static_cast<E &&>(err.value)}
  {
  }

  constexpr Result(Result && other) : is_ok_{other.is_ok_}
  {
    if (other.is_ok_)
    {
      new (&value_) T{static_cast<T &&>(other.value_)};
    }
    else
    {
      new (&err_) E{static_cast<E &&>(other.err_)};
    }
  }

  constexpr Result & operator=(Result && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      err_.~E();
    }

    is_ok_ = other.is_ok_;

    if (other.is_ok_)
    {
      new (&value_) T{static_cast<T &&>(other.value_)};
    }
    else
    {
      new (&err_) E{static_cast<E &&>(other.err_)};
    }

    return *this;
  }

  constexpr Result & operator=(Ok<T> other)
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      err_.~E();
    }

    is_ok_ = true;
    new (&value_) T{static_cast<T &&>(other.value)};
    return *this;
  }

  constexpr Result & operator=(Err<T> other)
  {
    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      err_.~E();
    }

    is_ok_ = false;
    new (&err_) E{static_cast<E &&>(other.value)};
    return *this;
  }

  constexpr Result(Result const & other) : is_ok_{other.is_ok_}
  {
    if (other.is_ok_)
    {
      new (&value_) T{other.value_};
    }
    else
    {
      new (&err_) E{other.err_};
    }
  }

  constexpr Result & operator=(Result const & other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    if (is_ok_)
    {
      value_.~T();
    }
    else
    {
      err_.~E();
    }

    is_ok_ = other.is_ok_;

    if (other.is_ok_)
    {
      new (&value_) T{other.value_};
    }
    else
    {
      new (&err_) E{other.err_};
    }

    return *this;
  }

  [[nodiscard]] constexpr bool is_ok() const
  {
    return is_ok_;
  }

  [[nodiscard]] constexpr bool is_err() const
  {
    return !is_ok_;
  }

  [[nodiscard]] explicit constexpr operator bool() const
  {
    return is_ok_;
  }

  template <typename U>
  [[nodiscard]] constexpr bool contains(U const & cmp) const
  {
    if (is_ok_)
    {
      return value_ == cmp;
    }
    return false;
  }

  template <typename F>
  [[nodiscard]] constexpr bool contains_err(F const & cmp) const
  {
    if (is_ok_)
    {
      return false;
    }
    return err_ == cmp;
  }

  constexpr T & value(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_ok_, ".value() called on Result with Err = ", err_);
    return value_;
  }

  constexpr T const &
      value(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_DESC_SRC(loc, is_ok_, ".value() called on Result with Err = ", err_);
    return value_;
  }

  constexpr E & err(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, !is_ok_,
                   ".err() called on Result with Value = ", value_);
    return err_;
  }

  constexpr E const & err(SourceLocation loc = SourceLocation::current()) const
  {
    CHECK_DESC_SRC(loc, !is_ok_,
                   ".err() called on Result with Value = ", value_);
    return err_;
  }

  constexpr Result<T const *, E const *> as_ptr() const
  {
    if (is_ok_)
    {
      return Ok{&value_};
    }
    return Err{&err_};
  }

  constexpr Result<T *, E *> as_ptr()
  {
    if (is_ok_)
    {
      return Ok{&value_};
    }
    return Err{&err_};
  }

  template <typename Fn>
  constexpr auto map(Fn && op)
  {
    using U = decltype(static_cast<Fn &&>(op)(value_));
    if (is_ok_)
    {
      return Result<U, E>{Ok<U>{static_cast<Fn &&>(op)(value_)}};
    }
    return Result<U, E>{Err{err_}};
  }

  template <typename Fn, typename U>
  constexpr auto map_or(Fn && op, U && alt)
  {
    if (is_ok_)
    {
      return static_cast<Fn &&>(op)(value_);
    }
    return static_cast<U &&>(alt);
  }

  template <typename Fn, typename AltFn>
  constexpr decltype(auto) map_or_else(Fn && op, AltFn && alt_op)
  {
    if (is_ok_)
    {
      return static_cast<Fn &&>(op)(value_);
    }
    return static_cast<AltFn &&>(alt_op)(err_);
  }

  template <typename Fn>
  constexpr auto and_then(Fn && op)
  {
    using OutResult = decltype(static_cast<Fn &&>(op)(value_));
    if (is_ok_)
    {
      return static_cast<Fn &&>(op)(value_);
    }
    return OutResult{Err{err_}};
  }

  template <typename Fn>
  constexpr auto or_else(Fn && op)
  {
    using OutResult = decltype(static_cast<Fn &&>(op)(err_));
    if (is_ok_)
    {
      return OutResult{Ok{value_}};
    }
    return static_cast<Fn &&>(op)(err_);
  }

  template <typename U>
  constexpr T unwrap_or(U && alt)
  {
    if (is_ok_)
    {
      return static_cast<T &&>(value_);
    }
    return static_cast<U &&>(alt);
  }

  template <typename Fn>
  constexpr T unwrap_or_else(Fn && op)
  {
    if (is_ok_)
    {
      return static_cast<T &&>(value_);
    }
    return static_cast<Fn &&>(op)(err_);
  }

  constexpr T unwrap(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_ok_,
                   "Expected Value in Result but got Err = ", err_);
    return static_cast<T &&>(value_);
  }

  constexpr T expect(char const *   msg,
                     SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, is_ok_, msg, " ", err_);
    return static_cast<T &&>(value_);
  }

  constexpr E unwrap_err(SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, !is_ok_,
                   "Expected Err in Result but got Value = ", value_);
    return static_cast<E &&>(err_);
  }

  constexpr E expect_err(char const *   msg,
                         SourceLocation loc = SourceLocation::current())
  {
    CHECK_DESC_SRC(loc, !is_ok_, msg, " ", value_);
    return static_cast<E &&>(err_);
  }

  template <typename OkFn, typename ErrFn>
  constexpr decltype(auto) match(OkFn && ok_fn, ErrFn && err_fn)
  {
    if (is_ok_)
    {
      return ok_fn(value_);
    }
    return err_fn(err_);
  }

  template <typename OkFn, typename ErrFn>
  constexpr decltype(auto) match(OkFn && ok_fn, ErrFn && err_fn) const
  {
    if (is_ok_)
    {
      return ok_fn(value_);
    }
    return err_fn(err_);
  }
};

template <typename T, typename U>
[[nodiscard]] constexpr bool operator==(Ok<T> const & a, Ok<U> const & b)
{
  return a.value == b.value;
}

template <typename T, typename U>
[[nodiscard]] constexpr bool operator!=(Ok<T> const & a, Ok<U> const & b)
{
  return a.value != b.value;
}

template <typename E, typename F>
[[nodiscard]] constexpr bool operator==(Err<E> const & a, Err<F> const & b)
{
  return a.value == b.value;
}

template <typename E, typename F>
[[nodiscard]] constexpr bool operator!=(Err<E> const & a, Err<F> const & b)
{
  return a.value != b.value;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator==(Result<T, E> const & a, Ok<U> const & b)
{
  if (a.is_ok_)
  {
    return a.value_ == b.value;
  }
  return false;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const & a, Ok<U> const & b)
{
  if (a.is_ok_)
  {
    return a.value_ != b.value;
  }
  return true;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator==(Ok<U> const & a, Result<T, E> const & b)
{
  if (b.is_ok_)
  {
    return a.value == b.value_;
  }
  return false;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator!=(Ok<U> const & a, Result<T, E> const & b)
{
  if (b.is_ok_)
  {
    return a.value != b.value_;
  }
  return true;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator==(Result<T, E> const & a,
                                        Err<U> const &       b)
{
  if (a.is_err())
  {
    return a.err_ == b.value;
  }
  return false;
}

template <typename T, typename E, typename U>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const & a,
                                        Err<U> const &       b)
{
  if (a.is_err())
  {
    return a.err_ != b.value;
  }
  return true;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator==(Err<U> const &       a,
                                        Result<T, E> const & b)
{
  if (b.is_err())
  {
    return a.value == b.err_;
  }
  return false;
}

template <typename U, typename T, typename E>
[[nodiscard]] constexpr bool operator!=(Err<U> const &       a,
                                        Result<T, E> const & b)
{
  if (b.is_err())
  {
    return a.value != b.err_;
  }
  return true;
}

template <typename T, typename E, typename U, typename F>
[[nodiscard]] constexpr bool operator==(Result<T, E> const & a,
                                        Result<U, F> const & b)
{
  if (a.is_ok_ && b.is_ok_)
  {
    return a.value_ == b.value_;
  }
  if (a.is_err() && b.is_err())
  {
    return a.err_ == b.err_;
  }
  return false;
}

template <typename T, typename E, typename U, typename F>
[[nodiscard]] constexpr bool operator!=(Result<T, E> const & a,
                                        Result<U, F> const & b)
{
  if (a.is_ok_ && b.is_ok_)
  {
    return a.value_ != b.value_;
  }
  if (a.is_err() && b.is_err())
  {
    return a.err_ != b.err_;
  }
  return true;
}
}        // namespace ash
