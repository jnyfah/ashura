#pragma once

#include "ashura/std/op.h"
#include "ashura/std/types.h"
#include "ashura/std/vec.h"

namespace ash
{

// TODO(lamarrr): generational counters
///
/// @index_to_id: id of data, ordered relative to {data}
/// @id_to_index: map of id to index in {data}
/// @size: the number of valid elements in the sparse set
/// @capacity: the number of elements the sparse set has capacity for, includes
/// reserved but unallocated ids pointing to valid but uninitialized memory
///
/// The index and id either point to valid indices/ids or are an implicit free
/// list of ids and indices masked by RELEASE_MASK
///
///
/// Bits[0  : 47] -> index id
/// Bits[48 : 61] -> generational counter
/// Bits[62 : 63] -> release mask
///
///
struct SparseVec
{
  static constexpr uid STUB         = U64_MAX;
  static constexpr uid RELEASE_MASK = ~(STUB >> 1);
  static constexpr u64 MAX_ELEMENTS = STUB >> 1;
  static constexpr uid MAX_ID       = MAX_ELEMENTS;
  // TODO(lamarrr): use 15 bits for generational mask

  Vec<uid> index_to_id  = {};
  Vec<u64> id_to_index  = {};
  u64      free_id_head = STUB;

  [[nodiscard]] constexpr bool is_empty() const
  {
    return size() == 0;
  }

  [[nodiscard]] constexpr u64 *data() const
  {
    return index_to_id.data();
  }

  [[nodiscard]] constexpr u64 size() const
  {
    return static_cast<u64>(index_to_id.size());
  }

  [[nodiscard]] constexpr u64 *begin() const
  {
    return index_to_id.begin();
  }

  [[nodiscard]] constexpr u64 *end() const
  {
    return index_to_id.end();
  }

  constexpr operator Span<u64>() const
  {
    return index_to_id;
  }

  template <typename... VecT>
  constexpr void clear(VecT &...dense)
  {
    (dense.clear(), ...);
    id_to_index.clear();
    index_to_id.clear();
    free_id_head = STUB;
  }

  template <typename... VecT>
  constexpr void reset(VecT &...dense)
  {
    (dense.reset(), ...);
    id_to_index.reset();
    index_to_id.reset();
    free_id_head = STUB;
  }

  [[nodiscard]] constexpr bool is_valid_id(uid id) const
  {
    return id < id_to_index.size() && !(id_to_index[id] & RELEASE_MASK);
  }

  [[nodiscard]] constexpr bool is_valid_index(u64 index) const
  {
    return index < size();
  }

  [[nodiscard]] constexpr u64 operator[](uid id) const
  {
    return id_to_index[id];
  }

  [[nodiscard]] constexpr u64 to_index(uid id) const
  {
    return id_to_index[id];
  }

  [[nodiscard]] constexpr bool try_to_index(uid id, u64 &index) const
  {
    if (!is_valid_id(id))
    {
      return false;
    }

    index = to_index(id);
    return true;
  }

  [[nodiscard]] constexpr uid to_id(u64 index) const
  {
    return index_to_id[index];
  }

  [[nodiscard]] constexpr bool try_to_id(u64 index, uid &id) const
  {
    if (!is_valid_index(index))
    {
      return false;
    }

    id = to_id(id);
    return true;
  }

  template <typename VecT>
  constexpr bool try_get(uid id, VecT::Iterator &iterator, VecT &vec)
  {
    u64 index;
    if (!try_to_index(id, index))
    {
      return false;
    }
    iterator = vec.begin() + index;
    return true;
  }

  template <typename... VecT>
  constexpr void erase(uid id, VecT &...dense)
  {
    u64 const index = id_to_index[id];
    u64 const last  = size() - 1;

    if (index != last)
    {
      (dense.swap(index, last), ...);
    }

    (dense.pop(), ...);

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

  template <typename... VecT>
  [[nodiscard]] constexpr bool try_erase(uid id, VecT &...dense)
  {
    if (!is_valid_id(id))
    {
      return false;
    }
    erase(id, dense...);
    return true;
  }

  template <typename... VecT>
  [[nodiscard]] bool reserve(u64 target_capacity, VecT &...dense)
  {
    return ((id_to_index.reserve(target_capacity) &&
             index_to_id.reserve(target_capacity)) &&
            ... && dense.reserve(target_capacity));
  }

  template <typename... VecT>
  [[nodiscard]] bool grow(u64 target_size, VecT &...dense)
  {
    return ((id_to_index.grow(target_size) && index_to_id.grow(target_size)) &&
            ... && dense.grow(target_size));
  }

  /// make new id and map the unique id to the unique index
  [[nodiscard]] bool make_id(u64 index, uid &out_id)
  {
    if (free_id_head != STUB)
    {
      out_id              = free_id_head;
      id_to_index[out_id] = index;
      free_id_head        = ~RELEASE_MASK & id_to_index[free_id_head];
      return true;
    }
    else
    {
      if (!id_to_index.push(index))
      {
        return false;
      }
      out_id = static_cast<uid>(id_to_index.size() - 1);
      return true;
    }
  }

  template <typename PushOp, typename... VecT>
  [[nodiscard]] bool push(PushOp &&push_op, VecT &...dense)
  {
    u64 const index = size();
    uid       id;

    if (!(grow(size() + 1, dense...) && make_id(index, id) &&
          index_to_id.push(id)))
    {
      return false;
    }

    push_op(id, index);

    return true;
  }
};

}        // namespace ash
