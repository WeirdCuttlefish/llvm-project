/*
Single Function. If Control 2.
Output: 
WARNING! b is not updated! This error is at: 15:5
*/

int main(){
    int a = 0;
    int b = a;
    if (true){
        2;
    } else {
        a=2;
    } 
    b;
    return 1;
}