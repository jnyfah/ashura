/// SPDX-License-Identifier: MIT
#pragma once

#include "ashura/std/allocator.h"
#include "ashura/std/mem.h"
#include "ashura/std/obj.h"
#include "ashura/std/result.h"
#include "ashura/std/traits.h"
#include "ashura/std/types.h"

namespace ash
{

template <typename T>
requires (NonConst<T>)
struct [[nodiscard]] Vec
{
  using Type = T;
  using Repr = T;

  T *           storage_   = nullptr;
  usize         size_      = 0;
  usize         capacity_  = 0;
  AllocatorImpl allocator_ = {};

  explicit constexpr Vec(AllocatorImpl allocator) :
      storage_{nullptr},
      size_{0},
      capacity_{0},
      allocator_{allocator}
  {
  }

  constexpr Vec() : Vec{default_allocator}
  {
  }

  constexpr Vec(AllocatorImpl allocator, T * storage, usize capacity,
                usize size) :
      storage_{storage},
      size_{size},
      capacity_{capacity},
      allocator_{allocator}
  {
  }

  constexpr Vec(Vec const &) = delete;

  constexpr Vec & operator=(Vec const &) = delete;

  constexpr Vec(Vec && other) :
      storage_{other.storage_},
      size_{other.size_},
      capacity_{other.capacity_},
      allocator_{other.allocator_}
  {
    other.storage_   = nullptr;
    other.size_      = 0;
    other.capacity_  = 0;
    other.allocator_ = {};
  }

  constexpr Vec & operator=(Vec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) Vec{static_cast<Vec &&>(other)};
    return *this;
  }

  constexpr ~Vec()
  {
    uninit();
  }

  static constexpr Result<Vec> make(usize         capacity,
                                    AllocatorImpl allocator = {})
  {
    T * storage;
    if (!allocator.nalloc(capacity, storage)) [[unlikely]]
    {
      return Err{};
    }

    return Ok{
        Vec{allocator, storage, capacity, 0}
    };
  }

  constexpr Result<Vec> clone(AllocatorImpl allocator) const
  {
    Vec out{allocator};

    if (!out.extend_copy(*this))
    {
      return Err{};
    }

    return Ok{static_cast<Vec &&>(out)};
  }

  constexpr Result<Vec<T>> clone() const
  {
    return clone(allocator_);
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T * data() const
  {
    return storage_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  constexpr u32 size32() const
  {
    return (u32) size_;
  }

  constexpr u64 size64() const
  {
    return (u64) size_;
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr T * begin() const
  {
    return data();
  }

  constexpr T * end() const
  {
    return data() + size_;
  }

  constexpr T & first() const
  {
    return get(0);
  }

  constexpr T & last() const
  {
    return get(size_ - 1);
  }

  constexpr T & operator[](usize index) const
  {
    return get(index);
  }

  constexpr T & get(usize index) const
  {
    return data()[index];
  }

  constexpr T * try_get(usize index) const
  {
    if (index < size_) [[unlikely]]
    {
      return data() + index;
    }

    return nullptr;
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args) const
  {
    data()[index] = T{static_cast<Args &&>(args)...};
  }

  constexpr void clear()
  {
    obj::destruct(Span{data(), size_});
    size_ = 0;
  }

  constexpr void uninit()
  {
    obj::destruct(Span{data(), size_});
    allocator_.ndealloc(storage_, capacity_);
  }

  constexpr void reset()
  {
    uninit();
    storage_   = nullptr;
    size_      = 0;
    capacity_  = 0;
    allocator_ = default_allocator;
  }

  constexpr Result<> reserve(usize target_capacity)
  {
    if (capacity_ >= target_capacity)
    {
      return Ok{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      if (!allocator_.nrealloc(capacity_, target_capacity, storage_))
          [[unlikely]]
      {
        return Err{};
      }
    }
    else
    {
      T * new_storage;
      if (!allocator_.nalloc(target_capacity, new_storage)) [[unlikely]]
      {
        return Err{};
      }

      obj::relocate_non_overlapping(Span{data(), size_}, new_storage);
      allocator_.ndealloc(storage_, capacity_);
      storage_ = new_storage;
    }

    capacity_ = target_capacity;
    return Ok{};
  }

  constexpr Result<> fit()
  {
    if (size_ == capacity_)
    {
      return Ok{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      if (!allocator_.nrealloc(capacity_, size_, storage_)) [[unlikely]]
      {
        return Err{};
      }
    }
    else
    {
      T * new_storage;
      if (!allocator_.nalloc(size_, new_storage)) [[unlikely]]
      {
        return Err{};
      }

      obj::relocate_non_overlapping(Span{data(), size_}, new_storage);
      allocator_.ndealloc(storage_, capacity_);
      storage_ = new_storage;
    }

    capacity_ = size_;
    return Ok{};
  }

  constexpr Result<> grow(usize target_size)
  {
    if (capacity_ >= target_size)
    {
      return Ok{};
    }

    return reserve(max(target_size, capacity_ + (capacity_ >> 1)));
  }

  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  constexpr void erase(Slice slice)
  {
    slice = slice(size_);
    if constexpr (TriviallyRelocatable<T>)
    {
      mem::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());
    }
    else
    {
      obj::move_assign(Span{data() + slice.end(), size_ - slice.end()},
                       data() + slice.begin());

      obj::destruct(Span{data() + size_ - slice.span, slice.span});
    }
    size_ -= slice.span;
  }

  template <typename... Args>
  constexpr Result<> push(Args &&... args)
  {
    if (!grow(size_ + 1)) [[unlikely]]
    {
      return Err{};
    }

    new (storage_ + size_) T{static_cast<Args &&>(args)...};

    size_++;

    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    obj::destruct(Span{data() + size_ - num, num});
    size_ -= num;
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (size_ < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);

    return Ok{};
  }

  Result<> shift_uninit(usize first, usize distance)
  {
    first = min(first, size_);
    if (!grow(size_ + distance)) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      // potentially overlapping
      mem::move(Span{data() + first, size_ - first}, data() + first + distance);
    }
    else
    {
      usize const tail_first = max(first, min(size_, distance) - size_);

      // move construct tail elements to uninitialized placements
      obj::move_construct(Span{data() + tail_first, size_ - tail_first},
                          data() + tail_first + distance);

      // move non-tail elements towards end
      obj::move_assign(Span{data() + first, tail_first - first},
                       data() + first + distance);

      // destruct previous placements of non-tail elements
      obj::destruct(Span{data() + first, tail_first - first});
    }

    size_ += distance;

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<> insert(usize pos, Args &&... args)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, 1)) [[unlikely]]
    {
      return Err{};
    }

    new (storage_ + pos) T{static_cast<Args &&>(args)...};
    return Ok{};
  }

  constexpr Result<> insert_span_copy(usize pos, Span<T const> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> insert_span_move(usize pos, Span<T> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if (!grow(size_ + extension)) [[unlikely]]
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend_defaulted(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    obj::default_construct(Span{data() + pos, extension});

    return Ok{};
  }

  constexpr Result<> extend_copy(Span<T const> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_move(Span<T> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // non-overlapping, use memcpy
    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr void swap(usize a, usize b) const
  {
    ash::swap(data()[a], data()[b]);
  }

  constexpr Result<> resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - size_);
  }

  constexpr Result<> resize_defaulted(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_defaulted(new_size - size_);
  }

  constexpr Span<T> span() const
  {
    return Span<T>{data(), size()};
  }
};

template <typename T>
constexpr Result<Vec<T>> vec(usize capacity, AllocatorImpl allocator = {})
{
  return Vec<T>::make(capacity, allocator);
}

template <typename T, usize N>
constexpr Result<Vec<T>> vec(T (&data)[N], AllocatorImpl allocator = {})
{
  Result out = Vec<T>::make(N, allocator);

  if (!out)
  {
    return out;
  }

  out.value().extend_copy(data).unwrap();

  return out;
}

template <typename T>
constexpr Result<Vec<T>> vec(Span<T const> data, AllocatorImpl allocator = {})
{
  Result out = Vec<T>::make(data.size(), allocator);

  if (!out)
  {
    return out;
  }

  out.value().extend_copy(data).unwrap();

  return out;
}

template <typename T, usize N>
constexpr Result<Vec<T>> vec_move(T (&&data)[N], AllocatorImpl allocator = {})
{
  Result out = Vec<T>::make(N, allocator);

  if (!out)
  {
    return out;
  }

  out.value().extend_move(data).unwrap();

  return out;
}

template <typename T>
constexpr Result<Vec<T>> vec_move(Span<T> data, AllocatorImpl allocator = {})
{
  Result out = Vec<T>::make(data.size(), allocator);

  if (!out)
  {
    return out;
  }

  out.value().extend_move(data).unwrap();

  return out;
}

/// @brief A vector with elements pinned to memory, The address of the vector is
/// stable over its lifetime and guarantees that there's never reference
/// invalidation. The elements are never relocated during their lifetime. It can
/// only pop elements and add elements while within its capacity. It also never
/// reallocates nor grow in capacity.
template <typename T>
requires (NonConst<T>)
struct [[nodiscard]] PinVec
{
  T *           storage_;
  usize         size_;
  usize         capacity_;
  AllocatorImpl allocator_;

  constexpr PinVec() :
      storage_{nullptr},
      size_{0},
      capacity_{0},
      allocator_{default_allocator}
  {
  }

  constexpr PinVec(AllocatorImpl allocator, T * storage, usize capacity,
                   usize size) :
      storage_{storage},
      size_{size},
      capacity_{capacity},
      allocator_{allocator}
  {
  }

  constexpr PinVec(PinVec const &) = delete;

  constexpr PinVec & operator=(PinVec const &) = delete;

  constexpr PinVec(PinVec && other) :
      storage_{other.storage_},
      size_{other.size_},
      capacity_{other.capacity_},
      allocator_{other.allocator_}
  {
    other.storage_   = nullptr;
    other.size_      = 0;
    other.capacity_  = 0;
    other.allocator_ = default_allocator;
  }

  constexpr PinVec & operator=(PinVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }

    uninit();
    new (this) PinVec{static_cast<PinVec &&>(other)};

    return *this;
  }

  constexpr ~PinVec()
  {
    uninit();
  }

  static constexpr Result<PinVec> make(usize         capacity,
                                       AllocatorImpl allocator = {})
  {
    T * storage;
    if (!allocator.nalloc(capacity, storage)) [[unlikely]]
    {
      return Err{};
    }

    return Ok{
        PinVec<T>{allocator, storage, capacity, 0}
    };
  }

  constexpr void uninit()
  {
    obj::destruct(Span{data(), size_});
    allocator_.ndealloc(storage_, capacity_);
  }

  constexpr void reset()
  {
    uninit();
    storage_   = nullptr;
    size_      = 0;
    capacity_  = 0;
    allocator_ = {};
  }

  constexpr Result<PinVec> clone(AllocatorImpl allocator) const
  {
    Result<PinVec> out = PinVec::make(allocator, capacity_);

    if (!out)
    {
      return out;
    }

    obj::copy_construct(span(), out.value().span());

    return out;
  }

  Result<PinVec> clone() const
  {
    return clone(allocator_);
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T * data() const
  {
    return storage_;
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  constexpr u32 size32() const
  {
    return (u32) size_;
  }

  constexpr u64 size64() const
  {
    return (u64) size_;
  }

  constexpr usize capacity() const
  {
    return capacity_;
  }

  constexpr T * begin() const
  {
    return data();
  }

  constexpr T * end() const
  {
    return data() + size_;
  }

  constexpr T & first() const
  {
    return get(0);
  }

  constexpr T & last() const
  {
    return get(size_ - 1);
  }

  constexpr T & operator[](usize index) const
  {
    return get(index);
  }

  constexpr T & get(usize index) const
  {
    return data()[index];
  }

  constexpr void clear()
  {
    obj::destruct(Span{data(), size_});
    size_ = 0;
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    obj::destruct(Span{data() + size_ - num, num});
    size_ -= num;
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (size_ < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<> push(Args &&... args)
  {
    if ((size_ + 1) > capacity_) [[unlikely]]
    {
      return Err{};
    }

    new (storage_ + size_) T{static_cast<Args &&>(args)...};

    size_++;

    return Ok{};
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if ((size_ + extension) > capacity_) [[unlikely]]
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend_defaulted(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    obj::default_construct(Span{data() + pos, extension});

    return Ok{};
  }

  constexpr Result<> extend_copy(Span<T const> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // free to use memcpy because the source range is not overlapping with this
    // anyway
    if constexpr (TriviallyCopyConstructible<T>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_move(Span<T> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    // non-overlapping, use memcpy
    if constexpr (TriviallyMoveConstructible<T>)
    {
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Span<T> span() const
  {
    return Span<T>{data(), size()};
  }
};

template <typename T>
constexpr Result<PinVec<T>> pin_vec(usize         capacity,
                                    AllocatorImpl allocator = {})
{
  return PinVec<T>::make(capacity, allocator);
}

template <typename R>
requires (NonConst<R>)
struct [[nodiscard]] BitVec
{
  using Type = bool;
  using Repr = R;

  Vec<R> repr_     = {};
  usize  bit_size_ = 0;

  explicit constexpr BitVec(AllocatorImpl allocator) : repr_{allocator}
  {
  }

  constexpr BitVec() : BitVec{default_allocator}
  {
  }

  constexpr BitVec(BitVec const &) = delete;

  constexpr BitVec & operator=(BitVec const &) = delete;

  constexpr BitVec(BitVec && other) :
      repr_{static_cast<Vec<R> &&>(other.repr_)},
      bit_size_{other.bit_size_}
  {
    other.bit_size_ = 0;
  }

  constexpr BitVec & operator=(BitVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) BitVec{static_cast<BitVec &&>(other)};
    return *this;
  }

  constexpr ~BitVec() = default;

  constexpr Vec<R> const & repr() const
  {
    return repr_;
  }

  constexpr Vec<R> & repr()
  {
    return repr_;
  }

  constexpr usize size() const
  {
    return bit_size_;
  }

  constexpr bool is_empty() const
  {
    return bit_size_ == 0;
  }

  constexpr bool has_trailing() const
  {
    return bit_size_ != (repr_.size_ * sizeof(R) * 8);
  }

  constexpr usize capacity() const
  {
    return repr_.capacity_ * sizeof(R) * 8;
  }

  constexpr void clear()
  {
    repr_.clear();
    bit_size_ = 0;
  }

  constexpr void uninit()
  {
    repr_.uninit();
  }

  constexpr void reset()
  {
    repr_.reset();
    bit_size_ = 0;
  }

  constexpr bool operator[](usize index) const
  {
    return get(index);
  }

  constexpr bool first() const
  {
    return get(0);
  }

  constexpr bool last() const
  {
    return get(bit_size_ - 1);
  }

  constexpr bool get(usize index) const
  {
    return ash::get_bit(repr_.span(), index);
  }

  constexpr void set(usize index, bool value) const
  {
    ash::assign_bit(repr_.span(), index, value);
  }

  constexpr bool get_bit(usize index) const
  {
    return get(index);
  }

  constexpr bool set_bit(usize index) const
  {
    return ash::set_bit(repr_.span(), index);
  }

  constexpr bool clear_bit(usize index) const
  {
    return ash::clear_bit(repr_.span(), index);
  }

  constexpr void flip_bit(usize index) const
  {
    ash::flip_bit(repr_.span(), index);
  }

  constexpr Result<> reserve(usize target_capacity)
  {
    return repr_.reserve(bit_packs<R>(target_capacity));
  }

  constexpr Result<> fit()
  {
    return repr_.fit();
  }

  constexpr Result<> grow(usize target_size)
  {
    return repr_.grow(bit_packs<R>(target_size));
  }

  constexpr Result<> push(bool bit)
  {
    usize const index = bit_size_;

    if (!extend_uninit(1)) [[unlikely]]
    {
      return Err{};
    }

    set(index, bit);
    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(bit_size_, num);
    bit_size_ -= num;
    usize const diff = repr_.size() - bit_packs<R>(bit_size_);
    repr_.pop(diff);
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (bit_size_ < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);
    return Ok{};
  }

  constexpr Result<> insert(usize pos, bool value)
  {
    pos = min(pos, bit_size_);
    if (!extend_uninit(1)) [[unlikely]]
    {
      return Err{};
    }
    for (usize src = pos, dst = src + 1; src < bit_size_; src++, dst++)
    {
      set(dst, get(src));
    }
    set(pos, value);
    return Ok{};
  }

  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  constexpr void erase(Slice slice)
  {
    slice = slice(bit_size_);
    for (usize dst = slice.begin(), src = slice.end(); src != bit_size_;
         dst++, src++)
    {
      set(dst, get(src));
    }
    pop(slice.span);
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if (!repr_.extend_uninit(bit_packs<R>(bit_size_ + extension) -
                             bit_packs<R>(bit_size_))) [[unlikely]]
    {
      return Err{};
    }

    bit_size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend_defaulted(usize extension)
  {
    usize const pos = bit_size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    for (usize i = pos; i < bit_size_; i++)
    {
      set(i, false);
    }

    return Ok{};
  }

  constexpr Result<> resize_uninit(usize new_size)
  {
    if (new_size <= bit_size_)
    {
      erase(new_size, bit_size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - bit_size_);
  }

  constexpr Result<> resize_defaulted(usize new_size)
  {
    if (new_size <= bit_size_)
    {
      erase(new_size, bit_size_ - new_size);
      return Ok{};
    }

    return extend_defaulted(new_size - bit_size_);
  }

  constexpr void swap(usize a, usize b) const
  {
    bool av = get(a);
    bool bv = get(b);
    set(a, bv);
    set(b, av);
  }

  constexpr BitSpan<R> span() const
  {
    return BitSpan<R>{repr_, bit_size_};
  }
};

template <typename T, usize C>
requires (NonConst<T>)
struct [[nodiscard]] InplaceVec
{
  using Type = T;
  using Repr = T;

  static constexpr usize Capacity = C;

  // uninitialized storage
  alignas(T) u8 storage_[C * sizeof(T)];
  usize size_ = 0;

  constexpr InplaceVec() = default;

  constexpr InplaceVec(InplaceVec const & other) : size_{other.size_}
  {
    copy_construct(Span{other.data(), other.size_}, data());
  }

  constexpr InplaceVec & operator=(InplaceVec const & other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) InplaceVec{other};
    return *this;
  }

  constexpr InplaceVec(InplaceVec && other) : size_{other.size_}
  {
    obj::relocate_non_overlapping(Span{other.data(), other.size_}, data());
    other.size_ = 0;
  }

  constexpr InplaceVec & operator=(InplaceVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    uninit();
    new (this) InplaceVec{static_cast<InplaceVec &&>(other)};
    return *this;
  }

  constexpr ~InplaceVec()
  {
    uninit();
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr T * data()
  {
    return reinterpret_cast<T *>(storage_);
  }

  constexpr T const * data() const
  {
    return reinterpret_cast<T const *>(storage_);
  }

  constexpr usize size() const
  {
    return size_;
  }

  constexpr usize size_bytes() const
  {
    return sizeof(T) * size_;
  }

  constexpr u32 size32() const
  {
    return (u32) size_;
  }

  constexpr u64 size64() const
  {
    return (u64) size_;
  }

  static constexpr usize capacity()
  {
    return Capacity;
  }

  constexpr T * begin()
  {
    return data();
  }

  constexpr T const * begin() const
  {
    return data();
  }

  constexpr T * end()
  {
    return data() + size_;
  }

  constexpr T const * end() const
  {
    return data() + size_;
  }

  constexpr T & first()
  {
    return get(0);
  }

  constexpr T const & first() const
  {
    return get(0);
  }

  constexpr T & last()
  {
    return get(size_ - 1);
  }

  constexpr T const & last() const
  {
    return get(size_ - 1);
  }

  constexpr T & operator[](usize index)
  {
    return get(index);
  }

  constexpr T const & operator[](usize index) const
  {
    return get(index);
  }

  constexpr T & get(usize index)
  {
    return data()[index];
  }

  constexpr T const & get(usize index) const
  {
    return data()[index];
  }

  constexpr T * try_get(usize index)
  {
    if (index >= size_) [[unlikely]]
    {
      return nullptr;
    }

    return data() + index;
  }

  constexpr T const * try_get(usize index) const
  {
    if (index >= size_) [[unlikely]]
    {
      return nullptr;
    }

    return data() + index;
  }

  template <typename... Args>
  constexpr void set(usize index, Args &&... args)
  {
    data()[index] = T{static_cast<Args &&>(args)...};
  }

  constexpr void clear()
  {
    obj::destruct(Span{data(), size_});
    size_ = 0;
  }

  constexpr void uninit()
  {
    obj::destruct(Span{data(), size_});
  }

  constexpr void reset()
  {
    uninit();
    size_ = 0;
  }

  constexpr void erase(usize first, usize num)
  {
    return erase(Slice{first, num});
  }

  constexpr void erase(Slice slice)
  {
    slice = slice(size_);

    if constexpr (TriviallyRelocatable<T>)
    {
      mem::move(Span{data() + slice.end(), size_ - slice.end()},
                data() + slice.begin());
    }
    else
    {
      obj::move_assign(Span{data() + slice.end(), size_ - slice.end()},
                       data() + slice.begin());

      obj::destruct(Span{data() + size_ - slice.span, slice.span});
    }

    size_ -= slice.span;
  }

  template <typename... Args>
  constexpr Result<> push(Args &&... args)
  {
    if ((size_ + 1) > Capacity) [[unlikely]]
    {
      return Err{};
    }

    new (data() + size_) T{static_cast<Args &&>(args)...};

    size_++;

    return Ok{};
  }

  constexpr void pop(usize num = 1)
  {
    num = min(num, size_);
    obj::destruct(Span{data() + size_ - num, num});
    size_ -= num;
  }

  constexpr Result<> try_pop(usize num = 1)
  {
    if (size_ < num) [[unlikely]]
    {
      return Err{};
    }

    pop(num);

    return Ok{};
  }

  Result<> shift_uninit(usize first, usize distance)
  {
    first = min(first, size_);

    if ((size_ + distance) > Capacity) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyRelocatable<T>)
    {
      // potentially overlapping
      mem::move(Span{data() + first, size_ - first}, data() + first + distance);
    }
    else
    {
      usize const tail_first = max(first, min(size_, distance) - size_);

      // move construct tail elements to uninitialized placements
      obj::move_construct(Span{data() + tail_first, size_ - tail_first},
                          data() + tail_first + distance);

      // move non-tail elements towards end
      obj::move_assign(Span{data() + first, tail_first - first},
                       data() + first + distance);

      // destruct previous placements of non-tail elements
      obj::destruct(Span{data() + first, tail_first - first});
    }

    size_ += distance;

    return Ok{};
  }

  template <typename... Args>
  constexpr Result<> insert(usize pos, Args &&... args)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, 1)) [[unlikely]]
    {
      return Err{};
    }

    new (data() + pos) T{static_cast<Args &&>(args)...};
    return Ok{};
  }

  constexpr Result<> insert_span_copy(usize pos, Span<T const> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<T>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> insert_span_move(usize pos, Span<T> span)
  {
    pos = min(pos, size_);

    if (!shift_uninit(pos, span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<T>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_uninit(usize extension)
  {
    if ((size_ + extension) > Capacity) [[unlikely]]
    {
      return Err{};
    }

    size_ += extension;

    return Ok{};
  }

  constexpr Result<> extend_defaulted(usize extension)
  {
    usize const pos = size_;

    if (!extend_uninit(extension)) [[unlikely]]
    {
      return Err{};
    }

    obj::default_construct(Span{data() + pos, extension});

    return Ok{};
  }

  constexpr Result<> extend_copy(Span<T const> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyCopyConstructible<T>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::copy_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr Result<> extend_move(Span<T> span)
  {
    usize const pos = size_;

    if (!extend_uninit(span.size())) [[unlikely]]
    {
      return Err{};
    }

    if constexpr (TriviallyMoveConstructible<T>)
    {
      // non-overlapping, use memcpy
      mem::copy(span, data() + pos);
    }
    else
    {
      obj::move_assign(span, data() + pos);
    }

    return Ok{};
  }

  constexpr void swap(usize a, usize b)
  {
    ash::swap(data()[a], data()[b]);
  }

  constexpr Result<> resize_uninit(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_uninit(new_size - size_);
  }

  constexpr Result<> resize_defaulted(usize new_size)
  {
    if (new_size <= size_)
    {
      erase(new_size, size_ - new_size);
      return Ok{};
    }

    return extend_defaulted(new_size - size_);
  }

  constexpr Span<T> span()
  {
    return Span<T>{data(), size()};
  }

  constexpr Span<T const> span() const
  {
    return Span<T const>{data(), size()};
  }
};

/// @brief Sparse Vector (a.k.a Sparse Set) are used for stable ID-tagging of
/// objects in high-perf scenarious i.e. ECS, where a stable identity is needed
/// for objects and they need to be processed in batches for efficiency. They
/// have an indirect index into their elements, although they don't guarantee
/// stability of the addresses of the elements they guarantee that the IDs
/// persist until the id is released. Unlike typical Sparse Sets, Sparse Vec's
/// elements are always contiguous without holes in them, making them suitable
/// for operations like batch-processing and branchless SIMD.
///
/// @tparam V dense containers for the properties, i.e. Vec<i32>, Vec<f32>
/// @param index_to_id id of data, ordered relative to {data}
/// @param id_to_index map of id to index in {data}
/// @param size the number of valid elements in the sparse set
/// @param capacity the number of elements the sparse set has capacity for,
/// includes reserved but unallocated ids pointing to valid but uninitialized
/// memory
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASE_MASK
///
///
template <typename... V>
requires (true && ... && NonConst<V>)
struct SparseVec
{
  static constexpr u64 RELEASE_MASK = U64_MAX >> 1;
  static constexpr u64 STUB         = U64_MAX;

  using Dense = Tuple<V...>;
  using Id    = u64;
  using Ids   = Vec<u64>;

  Ids   index_to_id  = {};
  Ids   id_to_index  = {};
  Dense dense        = {};
  u64   free_id_head = STUB;

  explicit constexpr SparseVec(Ids index_to_id, Ids id_to_index, Dense dense,
                               u64 free_id_head) :
      index_to_id{static_cast<Ids &&>(index_to_id)},
      id_to_index{static_cast<Ids &&>(id_to_index)},
      dense{static_cast<Dense &&>(dense)},
      free_id_head{free_id_head}
  {
  }

  explicit constexpr SparseVec(AllocatorImpl allocator) :
      index_to_id{allocator},
      id_to_index{allocator},
      dense{},
      free_id_head{STUB}
  {
  }

  constexpr SparseVec() : SparseVec{default_allocator}
  {
  }

  constexpr SparseVec(SparseVec const &) = delete;

  constexpr SparseVec & operator=(SparseVec const &) = delete;

  constexpr SparseVec(SparseVec && other) :
      index_to_id{static_cast<Ids &&>(other.index_to_id)},
      id_to_index{static_cast<Ids &&>(other.id_to_index)},
      dense{static_cast<Dense &&>(other.dense)},
      free_id_head{other.free_id_head}
  {
    other.free_id_head = STUB;
  }

  constexpr SparseVec & operator=(SparseVec && other)
  {
    if (this == &other) [[unlikely]]
    {
      return *this;
    }
    index_to_id  = static_cast<Ids &&>(other.index_to_id);
    id_to_index  = static_cast<Ids &&>(other.id_to_index);
    dense        = static_cast<Dense &&>(other.dense);
    free_id_head = other.free_id_head;
    return *this;
  }

  constexpr ~SparseVec() = default;

  constexpr bool is_empty() const
  {
    return size() == 0;
  }

  constexpr u64 size() const
  {
    return static_cast<u64>(index_to_id.size());
  }

  constexpr void clear()
  {
    apply([](auto &... d) { (d.clear(), ...); }, dense);
    id_to_index.clear();
    index_to_id.clear();
    free_id_head = STUB;
  }

  constexpr void reset()
  {
    apply([](auto &... d) { (d.reset(), ...); }, dense);
    id_to_index.reset();
    index_to_id.reset();
    free_id_head = STUB;
  }

  constexpr void uninit()
  {
    apply([](auto &... d) { (d.uninit(), ...); }, dense);
    id_to_index.uninit();
    index_to_id.uninit();
  }

  constexpr bool is_valid_id(u64 id) const
  {
    return id < id_to_index.size() && !(id_to_index[id] & RELEASE_MASK);
  }

  constexpr bool is_valid_index(u64 index) const
  {
    return index < size();
  }

  constexpr u64 operator[](u64 id) const
  {
    return id_to_index[id];
  }

  constexpr u64 to_index(u64 id) const
  {
    return id_to_index[id];
  }

  constexpr Result<u64, Void> try_to_index(u64 id) const
  {
    if (!is_valid_id(id)) [[unlikely]]
    {
      return Err{};
    }

    return Ok{to_index(id)};
  }

  constexpr u64 to_id(u64 index) const
  {
    return index_to_id[index];
  }

  constexpr Result<u64, Void> try_to_id(u64 index) const
  {
    if (!is_valid_index(index)) [[unlikely]]
    {
      return Err{};
    }

    return Ok{to_id(index)};
  }

  constexpr void erase(u64 id)
  {
    u64 const index = id_to_index[id];
    u64 const last  = size() - 1;

    if (index != last)
    {
      apply([index, last](auto &... d) { (d.swap(index, last), ...); }, dense);
    }

    apply([](auto &... d) { (d.pop(), ...); }, dense);

    // adjust id and index mapping
    if (index != last)
    {
      id_to_index[index_to_id[last]] = index;
      index_to_id[index]             = index_to_id[last];
    }

    id_to_index[id] = free_id_head | RELEASE_MASK;
    free_id_head    = id;
    index_to_id.pop();
  }

  constexpr Result<> try_erase(u64 id)
  {
    if (!is_valid_id(id)) [[unlikely]]
    {
      return Err{};
    }
    erase(id);
    return Ok{};
  }

  constexpr Result<> reserve(u64 target_capacity)
  {
    if (!(id_to_index.reserve(target_capacity) &&
          index_to_id.reserve(target_capacity))) [[unlikely]]
    {
      return Err{};
    }

    bool failed = apply(
        [&](auto &... d) {
          return (false || ... || !d.reserve(target_capacity));
        },
        dense);

    if (failed) [[unlikely]]
    {
      return Err{};
    }

    return Ok{};
  }

  constexpr Result<> grow(u64 target_size)
  {
    if (!(id_to_index.grow(target_size) && index_to_id.grow(target_size)))
        [[unlikely]]
    {
      return Err{};
    }

    bool failed = apply(
        [target_size](auto &... d) {
          return (false || ... || !d.grow(target_size));
        },
        dense);

    if (failed) [[unlikely]]
    {
      return Err{};
    }

    return Ok{};
  }

  /// make new id and map the unique id to the unique index
  constexpr Result<u64, Void> make_id(u64 index)
  {
    if (free_id_head != STUB)
    {
      u64 id          = free_id_head;
      id_to_index[id] = index;
      free_id_head    = ~RELEASE_MASK & id_to_index[free_id_head];
      return Ok{id};
    }
    else
    {
      if (!id_to_index.push(index)) [[unlikely]]
      {
        return Err{};
      }
      u64 id = static_cast<u64>(id_to_index.size() - 1);
      return Ok{id};
    }
  }

  template <usize I = 0>
  struct Pusher
  {
    template <typename Tuple, typename Head, typename... Tail>
    static constexpr void push(Tuple & t, Head && head, Tail &&... tail)
    {
      get<I>(t).push(static_cast<Head &&>(head)).unwrap();
      if constexpr (sizeof...(tail) != 0)
      {
        Pusher<I + 1>::push(t, static_cast<Tail &&>(tail)...);
      }
    }
  };

  template <typename... Args>
  constexpr Result<u64, Void> push(Args &&... args)
      requires (sizeof...(Args) == sizeof...(V))
  {
    u64 const index = size();

    if (!grow(size() + 1)) [[unlikely]]
    {
      return Err{};
    }

    Result id = make_id(index);

    if (!id) [[unlikely]]
    {
      return Err{};
    }

    if (!index_to_id.push(id.unwrap())) [[unlikely]]
    {
      return Err{};
    }

    Pusher<0>::push(dense, static_cast<Args &&>(args)...);
    return id;
  }
};

template <typename T>
struct IsTriviallyRelocatable<Vec<T>>
{
  static constexpr bool value = true;
};

template <typename T>
struct IsTriviallyRelocatable<PinVec<T>>
{
  static constexpr bool value = true;
};

template <typename T>
struct IsTriviallyRelocatable<BitVec<T>>
{
  static constexpr bool value = TriviallyRelocatable<Vec<T>>;
};

template <typename T, usize C>
struct IsTriviallyRelocatable<InplaceVec<T, C>>
{
  static constexpr bool value = TriviallyRelocatable<T>;
};

template <typename... T>
struct IsTriviallyRelocatable<SparseVec<T...>>
{
  static constexpr bool value = (true && ... && TriviallyRelocatable<Vec<T>>);
};

namespace fmt
{
inline bool push(Context const & ctx, Spec const & spec, Vec<char> const & str)
{
  return push(ctx, spec, str.span());
}

inline bool push(Context const & ctx, Spec const & spec,
                 PinVec<char> const & str)
{
  return push(ctx, spec, str.span());
}

template <usize C>
inline bool push(Context const & ctx, Spec const & spec,
                 InplaceVec<char, C> const & str)
{
  return push(ctx, spec, str.span());
}
}        // namespace fmt

}        // namespace ash
