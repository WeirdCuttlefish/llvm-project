/*
Global Variable with Pure functions.
Output: 
WARNING! c is not updated! This error is at: 10:3
*/
int a;
  
int foo(){
  int b = a;
  int c = b;
  b = 3;
  c;
  return 0;
}

int main() {
  int b = 2;
  int c = b;
  foo();
  return 0;
}