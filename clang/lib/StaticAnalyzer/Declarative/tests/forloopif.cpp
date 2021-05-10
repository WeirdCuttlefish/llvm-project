int foo(int *x){
  return *x;
}

int main(){
  int local = 0;
  int hi = 1;
  int x[] = {1,2,3};
  for (int i=0; i<2; i++){
    if (false) continue;
    if (10 < hi) hi = x[i];
  }
  foo(&hi);
  foo(&local);
  return 0;
}
