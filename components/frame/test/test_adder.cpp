#include "gtest/gtest.h"
#include "adder.h"


TEST(GTestSuiteAdder, add)
{
  Adder adder = Adder();
  int test_value = adder.add(2, 4);
  EXPECT_FLOAT_EQ(test_value, 6.0);
}