#include <cstdio>
#include <string>

#include "Extra.hpp"
// #include "annotations.hpp"

template <typename T, int label>
using annotate = T;

#define ORANGE 1

class Example1
{
private:
  Extra extra;

public:
  // @OrangeShareable
  annotate<int, ORANGE> myConstant;

  int getValue()
  {
    return this->extra.getValue();
  }

  Example1() : extra()
  {
  }
};

// @OrangeMain
int main(int argc, char **argv)
{
  Example1 e;
  printf("Hello Example 1: %d\n", e.getValue());
}
