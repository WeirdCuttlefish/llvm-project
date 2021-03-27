/*
Global Pointer
Output:

TODO: Fix there should be no error because foo is never called
*/

int a = 3;
int b = a;
int *c = &b;

int foo() {
    a = 3;
    *c;
    return 0;
}

int main() {
    b = 3;
    *c;
    return 0;
}