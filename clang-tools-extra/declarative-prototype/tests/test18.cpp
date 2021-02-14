/*
Basic pointer 2.
Output:
WARNING! z is not updated! This error is at: 18:5
*/

int main() {
    int a = 2;
    int z = a;

    int *b = &a;
    int *c = &a;

    *b = 3;

    b;
    c;
    z;

    return 0;
}