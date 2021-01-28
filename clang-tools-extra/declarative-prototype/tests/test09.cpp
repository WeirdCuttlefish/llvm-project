/*
Global function with global variable modification in call and error with global varible dependent on other global variable.
Output: 
WARNING! a is not updated! This error is at: 18:5
*/

int z = 0;
int a = z;

int foo(int x){
    a;
    z = 2;
    return 2;
}

int main(){
    foo(3);
    a;
    return 1;
}