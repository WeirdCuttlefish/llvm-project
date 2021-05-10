int foo(int *x){
  return *x;
}

int main(){
  int x = 3;
  if (auto y = &x){
    if (false) *y = 4;
    foo(y);
  }
  return 0;
}
