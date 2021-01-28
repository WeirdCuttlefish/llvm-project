/*
Global function with global variable modification in call and error with local varible dependent on other global variable.
Output: 
WARNING! a is not updated! This error is at: 17:5
*/

int z = 0;

int foo(int x){
    z = 2;
    return 2;
}

int main(){
    int a = z;
    foo(3);
    a;
    return 1;
}