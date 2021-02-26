/*
Global Pointer
Output:

*/

int a = 3;
int b = a;
int *c = &b;

int main() {
    a = 3;
    *c;
    return 0;
}