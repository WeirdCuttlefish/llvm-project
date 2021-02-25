/*
Basic Pointer problem. We actually might not want this to be an error.
We might actually just want to ignore pointers.
Output: 
WARNING: b from pointer c not updated!
*/

int main(){
    int a = 2;
    int b = a;
    int *c = &b;
    a = 3;
    c;
    *c;
    return 1;
}