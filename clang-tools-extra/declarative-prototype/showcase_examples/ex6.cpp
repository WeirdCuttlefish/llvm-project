/*
If node has both pointer edges and normal edges out of it, ignore reassignment
*/

int main() {
    int b = 3;
    int c = b;
    int d = sizeof(int);
    int *a = &b + d;

    *a = 3;

    c;               // Cannot deduce anything. No affect on graph. No error.
    return 0;
}