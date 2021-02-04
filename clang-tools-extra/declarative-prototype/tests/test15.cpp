/*
Function call in variable declaration.
Output: 
WARNING! b is not updated! This error is at: 15:5
NOT WORKING
*/

int a = 3;
int foo(){
    return a;
}

int main(){
    int b = foo();
    a = 4;
    b;
    return 0;
}