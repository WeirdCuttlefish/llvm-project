/*
Basic Pointer problem. We actually might not want this to be an error.
We might actually just want to ignore pointers.
Output: 
UNSURE
*/

int main(){
    int a = 2;
    int *b = &a;
    a = 3;
    b;
    return 1;
}