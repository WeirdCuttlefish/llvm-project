/*
Global Pointer
Output:
WARNING: b from pointer c not updated!
*/

int a = 3;
int b = a;
int *c = &b;

int foo() {
    b = 3;
    *c;
}

int main() {
    a = 3;
    *c;
    return 0;
}