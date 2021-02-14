/*
Basic.
*/

int main() {
    int b = 3;
    int c = 4;
    int a = b + c;
    int *ap = &a;

    b = 5;

    // a = c;

    // a;               // Error occurs here
    *ap;             // Error here
    ap;              // Should not be an error
    return 0;

}