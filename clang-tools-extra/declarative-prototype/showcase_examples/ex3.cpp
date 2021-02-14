/*
Pointer with nonpointer dependencies.
*/

int main() {
    int b = 3;
    int c = sizeof(int);
    int *a = &b + c;

    b = 5;

    a;               // There is no error because this is referring to address
    return 0;
}