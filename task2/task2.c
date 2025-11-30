#include <stdio.h>
#include <stdlib.h>

typedef int (*op_fn)(int, int);

static int add(int a, int b) {
    puts("Adding 'a' and 'b'");
    return a + b;
}

static int sub(int a, int b) {
    puts("Subtracting 'b' from 'a'");
    return a - b;
}

static int mul(int a, int b) {
    puts("Multiplying 'a' and 'b'");
    return a * b;
}

static int divide_(int a, int b) {
    puts("Dividing 'a' by 'b'");
    return a / b;
}

static int quit_prog(int a, int b) {
    (void)a; (void)b;
    exit(0);
}

static int invalid(int a, int b) {
    (void)a; (void)b;
    puts("Invalid operation");
    return 0;
}

int main(void)
{
    int a = 6, b = 3;
    char choice;

    printf("Operand 'a' : %d | Operand 'b' : %d\n", a, b);
    printf("Specify the operation to perform (0 : add | 1 : subtract | 2 : Multiply | 3 : divide | 4 : exit): ");

    scanf(" %c", &choice);

    op_fn ops[256];
    for (int i = 0; i < 256; i++) ops[i] = invalid;

    ops[(unsigned char)'0'] = add;
    ops[(unsigned char)'1'] = sub;
    ops[(unsigned char)'2'] = mul;
    ops[(unsigned char)'3'] = divide_;
    ops[(unsigned char)'4'] = quit_prog;

    int x = ops[(unsigned char)choice](a, b);
    printf("x = %d\n", x);

    return 0;
}
