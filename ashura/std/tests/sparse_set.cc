#include "gtest/gtest.h"

#include "ashura/std/sparse_set.h"
#include <bitset>
#include <iostream>

using namespace ash;

TEST(SparseSetTest, Start)
{
  SparseSet<u64> set;
  ASSERT_TRUE(set.reserve_new_ids(heap_allocator, 100));
  ASSERT_EQ(set.num_slots, 100);
  ASSERT_EQ(set.num_free, 100);
  uid64 id;
  for (u32 i = 0; i < 100; i++)
  {
    ASSERT_TRUE(set.allocate_id(id));
    ASSERT_LT(id, set.num_slots);
    ASSERT_GE(id, 0);
  }
  ASSERT_FALSE(set.allocate_id(id));
  ASSERT_TRUE((set.release(id, [&](u64 a, u64 b) {
    ASSERT_EQ(a, set.num_valid() - 1);
    ASSERT_LT(a, set.num_valid());
    ASSERT_LT(b, set.num_valid());
  })));
}
