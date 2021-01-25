/*
Single Function. No Control.
Output: 
WARNING! y is not updated! This error is at: 11:12
*/

int main() {
    int x=2;
    int y=x;
    x++; 
    x=(x + y) - 3;
    return 0;
}