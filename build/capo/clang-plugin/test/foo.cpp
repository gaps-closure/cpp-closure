class Foo
{ 
private:
    __attribute__((cle_annotate("PURPLE")))
    int bar;

    int baz;
public:
    Foo() {}
    int get_bar(); 
};

int Foo::get_bar()
{
    return bar;
}

__attribute__((cle_annotate("ORANGE")))
int foo(int x)
{
    Foo f;

    __attribute__((cle_annotate("BLUE")))
    int y;

    return 10 * f.get_bar();
}

int main()
{
    foo(5);
    return 0;
}
