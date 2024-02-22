#pragma cle def ORANGE {"level":"orange",\
  "cdf": [\
    {"remotelevel":"green", \
     "direction": "egress", \
     "guarddirective": { "operation": "allow"}}\
  ] }
#pragma cle def GREEN {"level":"green",\
  "cdf": [\
    {"remotelevel":"orange", \
     "direction": "egress", \
     "guarddirective": { "operation": "allow"}}\
  ] }

#pragma cle def GET_FOO {"level":"orange",\
  "cdf": [\
    {"remotelevel":"green", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [["ORANGE"]], \
     "codtaints": ["ORANGE"], \
     "rettaints": ["ORANGE"] \
    }, \
    {"remotelevel":"orange", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [["ORANGE"]], \
     "codtaints": ["ORANGE"], \
     "rettaints": ["ORANGE"] \
    } \
  ] }

__attribute__((cle_annotate("GET_FOO"))) 
double get_foo(double x) {

    return x * x + 1;
}

int main() {
    __attribute__((cle_annotate("GREEN")))
    double x = 1.0; 
    double z = get_foo(x);

    return 0;
}
