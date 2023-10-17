#include <cstdio>
#include <string>

#include "Bar.hpp"

class Foo
{
public:
  //ORANGE
  int a;

  int getValue()
  {
    return this->a;
  }

  Foo() {
    a = 1;
  }

};


// ORANGE
int main(int argc, char **argv)
{
  Foo foo = Foo();
  // PURPLESHAREABLE
  Bar bar = Bar();
  int ret = foo.getValue() + bar.getValue();
  printf("Result is: %d\n", ret);
}
