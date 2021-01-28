/*
Global function with global variable modification in call parameter.
Output: 
WARNING! a is not updated! This error is at: 21:5
*/

int z = 0;

int foo(int x){
    z = 2;
    return 2;
}

int foo2(int x){
    return 2;
}

int main(){
    int a = z;
    foo2(foo(2));
    a;
    return 1;
}