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
     "argtaints": ["PURPLE_SHAREABLE"], \
     "codtaints": ["PURPLE_SHAREABLE", "TAG_RESPONSE_XD_Bar_getValue"], \
     "rettaints": ["PURPLE_SHAREABLE"] \
    }, \
    {"remotelevel":"purple", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": ["PURPLE_SHAREABLE"], \
     "codtaints": ["PURPLE_SHAREABLE"] \
     "rettaints": ["PURPLE_SHAREABLE"] \
    } \
  ] }

// What taint do we want to assign the "this" argument

#pragma cle def XD_Bar_Bar {"level":"purple",\
  "cdf": [\
    {"remotelevel":"orange", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["PURPLE_SHAREABLE", "TAG_REQUEST_Bar_Bar"], \
     "rettaints": [] \
    }, \
    {"remotelevel":"purple", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": ["PURPLE_SHAREABLE"], \
     "codtaints": ["PURPLE_SHAREABLE"], \
     "rettaints": ["PURPLE_SHAREABLE"] \
    } \
  ] }

using namespace cle;

class Bar
{
public:
  annotate<int, "PURPLE_SHAREABLE"> b;
  int c; // what are implications for this field, is it PURPLE NONSHAREABLE by default? 

  [[clang::annotate("XD_Bar_getValue")]]
  int getValue()
  {
    return b;
  }

  [[clang::annotate("XD_Bar_Bar")]]
  Bar() {
    b = 2;
  }

};

#endif /* _BAR_ */

// Should class definitions be available on all encalves? Maybe some but not all
// Consider specifying the levels where the class definition can be, another field 
// after level on top section of CLE

// The this during constructor initialization will be tainted TAG_REQUEST_.._.. if XD
// need to understand propagation of this tag to codtaints or elsewhere.