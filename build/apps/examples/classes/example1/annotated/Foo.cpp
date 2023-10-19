#include <cstdio>
#include <string>

#include "closure.hpp"
#include "Bar.hpp"

#pragma cle def ORANGE {"level":"orange"}

#pragma cle def ORANGE_CALLABLE {"level":"orange", \
  "cdf": [\
      {"remotelevel":"orange", \
      "direction": "bidirectional", \
      "guarddirective": { "operation": "allow"}, \
      "argtaints": [["ORANGE"], ["ORANGE"]], \
      "codtaints": ["ORANGE"], \
      "rettaints": ["ORANGE"] \
      } \
  ]}

using namespace cle;

class Foo
{
public:
  annotate<int, "ORANGE"> a;
  
  int getValue()
  {
    return a;
  }

  Foo() {
    a = 1;
  }

};


[[clang::annotate("ORANGE_CALLABLE")]]
int main(int argc, char **argv)
{
  Foo foo = Foo();
  Bar bar = Bar();
  int ret = foo.getValue() + bar.getValue();
  printf("Result is: %d\n", ret);
  return 0;
}
