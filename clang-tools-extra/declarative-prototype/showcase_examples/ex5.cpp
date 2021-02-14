/*
Dereferencing invalidates everything a points to that has pointer edge.
*/

int main() {
    int b = 3;
    int c = b;
    int *a = &b;

    *a = 6;

    c;               // Error occurs here
    return 0;
}