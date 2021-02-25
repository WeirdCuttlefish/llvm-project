/*
Basic pointer.
Output:

*/

int main() {
    int a = 3;
    int c = a;
    int *b = &c;
    int **d = &b;
    a = 4;
    *b;
    *(*d);
    return 0;
}
