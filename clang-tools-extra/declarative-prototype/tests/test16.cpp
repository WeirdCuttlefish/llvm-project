/*
Basic redeclaration.
Output:
WARNING! y is not updated! This error is at: 15:5
*/

int main() {
    int x=2;
    int a=3;
    int y=x;
    y = a;
    x++;
    y;
    a++;
    y;
    return 0;
}