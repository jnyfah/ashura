#pragma once
#include "ashura/std/types.h"

namespace ash
{

typedef struct Allocator_T *Allocator;

/// @allocate: allocate aligned memory. returns null if
/// failed.
/// @allocate_zeroed: like allocate but zeroes the allocated memory, this is
/// performed by the OS and can be faster. returns null if allocation failed.
/// @reallocate: free the previously allocated memory and return a new memory,
/// alignment is not guaranteed to be preserved. if an error occurs, the old
/// memory is not freed and null is returned. alignment must be same as the
/// alignment of the original allocated memory.
/// @deallocate: free the previously allocated memory.
///
/// @release: releases all allocated memory on the allocator.
///
///
/// REQUIREMENTS
/// =============
///
/// @alignment: must be a power of 2. UB if 0 or otherwise
///
struct AllocatorInterface
{
  void *(*allocate)(Allocator self, usize alignment, usize size) = nullptr;
  void *(*allocate_zeroed)(Allocator self, usize alignment,
                           usize size)                           = nullptr;
  void *(*reallocate)(Allocator self, usize alignment, void *memory,
                      usize old_size, usize new_size)            = nullptr;
  void (*deallocate)(Allocator self, usize alignment, void *memory,
                     usize size)                                 = nullptr;
};

struct AllocatorImpl
{
  Allocator                 self      = nullptr;
  AllocatorInterface const *interface = nullptr;

  [[nodiscard]] constexpr void *allocate(usize alignment, usize size) const
  {
    return interface->allocate(self, alignment, size);
  }

  template <typename T>
  [[nodiscard]] constexpr T *allocate_typed(usize num) const
  {
    return (T *) interface->allocate(self, alignof(T), sizeof(T) * num);
  }

  [[nodiscard]] constexpr void *allocate_zeroed(usize alignment,
                                                usize size) const
  {
    return interface->allocate_zeroed(self, alignment, size);
  }

  template <typename T>
  [[nodiscard]] constexpr T *allocate_zeroed_typed(usize num) const
  {
    return (T *) interface->allocate_zeroed(self, alignof(T), sizeof(T) * num);
  }

  [[nodiscard]] constexpr void *reallocate(usize alignment, void *memory,
                                           usize old_size, usize new_size) const
  {
    return interface->reallocate(self, alignment, memory, old_size, new_size);
  }

  template <typename T>
  [[nodiscard]] constexpr T *reallocate_typed(T *memory, usize old_num,
                                              usize new_num) const
  {
    return (T *) interface->reallocate(
        self, alignof(T), memory, sizeof(T) * old_num, sizeof(T) * new_num);
  }

  constexpr void deallocate(usize alignment, void *memory, usize size) const
  {
    interface->deallocate(self, alignment, memory, size);
  }

  template <typename T>
  constexpr void deallocate_typed(T *memory, usize num) const
  {
    interface->deallocate(self, alignof(T), memory, sizeof(T) * num);
  }
};

struct HeapInterface
{
  static void *allocate(Allocator self, usize alignment, usize size);
  static void *allocate_zeroed(Allocator self, usize alignment, usize size);
  static void *reallocate(Allocator self, usize alignment, void *memory,
                          usize old_size, usize new_size);
  static void  deallocate(Allocator self, usize alignment, void *memory,
                          usize size);
};

inline constexpr AllocatorInterface heap_interface{
    .allocate        = HeapInterface::allocate,
    .allocate_zeroed = HeapInterface::allocate_zeroed,
    .reallocate      = HeapInterface::reallocate,
    .deallocate      = HeapInterface::deallocate};

/// guarantees at least MAX_STANDARD_ALIGNMENT alignment.
inline constexpr AllocatorImpl heap_allocator{.self      = nullptr,
                                              .interface = &heap_interface};

inline constexpr ash::AllocatorImpl default_allocator = heap_allocator;

}        // namespace ash
