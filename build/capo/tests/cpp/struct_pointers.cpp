struct foo {
    int bar;
};

int main() {
    struct foo x;
    x.bar = 42;
    int * y = &x.bar;
}