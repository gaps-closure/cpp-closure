#pragma cle def GREEN { "level": "green" }

int main() {
    __attribute__((cle_annotate("GREEN")))
    int y = 0;
    int *x = &y;
    int *z = x;
    
    return 0;
}