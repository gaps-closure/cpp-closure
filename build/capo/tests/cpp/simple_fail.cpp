#pragma cle def GREEN { "level": "green" }
#pragma cle def ORANGE { "level": "orange" }

int main() {
    __attribute__((cle_annotate("GREEN")))
    int y = 0;

    __attribute__((cle_annotate("ORANGE")))
    int *x = &y;
    int *z = x;
    
    return 0;
}