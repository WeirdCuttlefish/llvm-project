int main(){

  int i;

  double dot = 0.0;
  int n = 0;
  int arr[10] = {};
  for (int i=0; i<10; ++i){
    dot += arr[i];
  }
  if (dot < 3) n++;
  
  return 0;

}
