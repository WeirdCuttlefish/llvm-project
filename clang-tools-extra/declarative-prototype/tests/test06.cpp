/*
Single Function. For Control 1.
Output: 
WARNING! b is not updated! This error is at: 12:5
*/
int main(){
    int a = 0;
    int b = a;
    for(int i=0; i<10; i++){
        a = 3;
    }
    b;
    return 1;
}