/*
Basic pointer.
Output:
WARNING! y is not updated! This error is at: 22:5
*/

int main() {
    int a = 3;
    int z = 4;
    int *b = &a;
    int *c = b;
    int **d = &c;
    int *e = *d;
    *e = a;
    *d = &z;
    *c = *b;
    return 0;
}