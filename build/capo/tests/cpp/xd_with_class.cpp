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

class Foo {
    
    int bar;

public:
    Foo() {
        bar = 0;
    }

    int get_bar() { return bar; }

    void set_bar(int b) { bar = b; }
};

__attribute__((cle_annotate("GET_FOO"))) 
double get_foo(double x) {

    Foo g;
    return x * x + 1;
}


int main() {
    __attribute__((cle_annotate("GREEN")))
    double x = 1.0; 
    double z = get_foo(x);

    Foo f;
    return 0;
}
