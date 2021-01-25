/*
Single Function. For Control 2.
Output: 
WARNING! b is not updated! This error is at: 13:5
*/

int main(){
    int a = 0;
    int b = a;
    for(int i=0; i<10; i++){
        a = i;
    }
    b;
    return 1;
}