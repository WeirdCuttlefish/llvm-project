#include<iostream>
int main(){

  int x = 2;
  int y = 3;
  int z = x + y;
  x = 4;
  z; // Error here
  int a = 3;
  int b = a;
  if (true){
    if (true){
      int m = a;
      a = 3;
      m;  // Error here
    }
  }
  b; // Error here
  int p = 3;
  int j = p;
  while (false){
    p = 4;
  }
  j; // Error here

  int m = 3;
  int r = m;
  for (int i=0; i<10; ++i){
    m = i;
  }
  r;  // Error here
  


  return 1;
}

