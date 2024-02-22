#pragma cle def GREEN { "level": "green" }

class Foo {

    __attribute__((cle_annotate("GREEN")))
    int bar;

public:
    Foo() {
        bar = 0;
    }

    int get_bar() { return bar; }

    void set_bar(int b) { bar = b; }
};

int main() {

    Foo f;
    return 0;
}
