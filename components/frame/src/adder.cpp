#include "adder.h"
#include "example/example.h"
#include <iostream>


int Adder::add(int a, int b)
{
  Example example = Example();
  int sub_val = example.sub(a, b);
  std::cout << sub_val << std::endl;

  return (a + b);
}
