/*
Basic pointer 3.
Output:
WARNING! y is not updated! This error is at: 22:5
*/

int main() {
    int a = 2;
    int r = 3;

    int *b = &a;
    int *c = &a;

    int y = *b + r;

    *b = 3;

    a;
    r;
    b;
    c;
    y;

    return 0;
}