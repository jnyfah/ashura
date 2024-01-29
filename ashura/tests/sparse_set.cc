#include "ashura/std/sparse_set.h"
#include "gtest/gtest.h"

TEST(SparseSetTest, Start)
{
  ash::SparseSet<ash::u64> set;
  set.compact([](ash::u64, ash::u64) {});
}
