#pragma cle def ORANGE { "level":"orange" }
#pragma cle def ORANGE_SHAREABLE {"level":"orange",\
  "cdf": [\
    {"remotelevel":"green", \
     "direction": "egress", \
     "guarddirective": { "operation": "allow"}}\
  ] }

#pragma cle def GET_FOO {"level":"orange",\
  "cdf": [\
    {"remotelevel":"orange", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [["ORANGE_SHAREABLE"]], \
     "codtaints": ["ORANGE", "ORANGE_SHAREABLE"], \
     "rettaints": ["ORANGE", "ORANGE_SHAREABLE"] \
    } \
  ] }


__attribute__((cle_annotate("ORANGE"))) 
int y = 42;

__attribute__((cle_annotate("ORANGE_SHAREABLE"))) 
int z = 54;


__attribute__((cle_annotate("GET_FOO"))) 
double get_foo(double x) {
  return z;
}

int main() {
  __attribute__((cle_annotate("ORANGE_SHAREABLE")))
  double x = 1.0; 
  double z = get_foo(x);

  return 0;
}
