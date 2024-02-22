#pragma cle def GREEN {"level":"green",\
  "cdf": [\
    {"remotelevel":"orange", \
     "direction": "egress", \
     "guarddirective": { "operation": "allow"}}\
  ] }

#pragma cle def ORANGE {"level":"orange",\
  "cdf": [\
    {"remotelevel":"green", \
     "direction": "egress", \
     "guarddirective": { "operation": "allow"}}\
  ] }

#pragma cle def GET_BAR {"level":"orange",\
  "cdf": [\
    {"remotelevel":"green", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["ORANGE"], \
     "rettaints": ["ORANGE"] \
    }, \
    {"remotelevel":"orange", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["ORANGE"], \
     "rettaints": ["ORANGE"] \
    } \
  ] }

#pragma cle def FOO {"level":"orange",\
  "cdf": [\
    {"remotelevel":"green", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["ORANGE"], \
     "rettaints": ["ORANGE"] \
    }, \
    {"remotelevel":"orange", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["ORANGE"], \
     "rettaints": ["ORANGE"] \
    } \
  ] }



class __attribute__((cle_annotate("ORANGE"))) Foo {
    
public:
    int bar;

    __attribute__((cle_annotate("FOO")))
    Foo() {
        bar = 0;
    }

    __attribute__((cle_annotate("GET_BAR")))
    int get_bar() { return bar; }

    void set_bar(int b) { bar = b; }
};


int main() {

    __attribute__((cle_annotate("GREEN")))
    int x = 0;

    Foo f = Foo();
    return f.bar;
}