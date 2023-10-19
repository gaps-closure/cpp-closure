#ifndef _BAR_
#define _BAR_

#include "closure.hpp"

#pragma cle def PURPLE_SHAREABLE {"level":"purple",\
  "cdf": [\
    {"remotelevel":"orange", \
     "direction": "egress", \
     "guarddirective": { "operation": "allow"}}\
  ] }

#pragma cle def XD_Bar_getValue {"level":"purple",\
  "cdf": [\
    {"remotelevel":"orange", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["PURPLE_SHAREABLE", "TAG_RESPONSE_XD_Bar_getValue"], \
     "rettaints": ["PURPLE_SHAREABLE"] \
    }, \
    {"remotelevel":"purple", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["PURPLE_SHAREABLE", "TAG_RESPONSE_XD_Bar_getValue"], \
     "rettaints": ["PURPLE_SHAREABLE"] \
    } \
  ] }

#pragma cle def XD_Bar_Bar {"level":"purple",\
  "cdf": [\
    {"remotelevel":"orange", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["PURPLE_SHAREABLE", "TAG_RESPONSE_XD_Bar_getValue"], \
     "rettaints": ["PURPLE_SHAREABLE"] \
    }, \
    {"remotelevel":"purple", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["PURPLE_SHAREABLE", "TAG_RESPONSE_XD_Bar_getValue"], \
     "rettaints": ["PURPLE_SHAREABLE"] \
    } \
  ] }

using namespace cle;

class Bar
{
public:
  annotate<int, "PURPLE_SHAREABLE"> b;

  [[clang::annotate("XD_Bar_getValue")]]
  int getValue()
  {
    return this->b;
  }

  [[clang::annotate("XD_Bar_Bar")]]
  Bar() {
    b = 2;
  }

};

#endif /* _BAR_ */