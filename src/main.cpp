#include "adder.h"
#include <iostream>


int main(){
  Adder adder = Adder();
  int added_value = adder.add(2, 4);

  std::cout << added_value << std::endl;

  return 0;
}