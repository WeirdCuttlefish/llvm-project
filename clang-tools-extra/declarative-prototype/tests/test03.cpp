/*
Single Function. If Control 1.
Output: 
WARNING! b is not updated! This error is at: 15:5
*/

int main(){
    int a = 0;
    int b = a;
    if (true){
        a = 2;
    } else {
        2;
    } 
    b; 
    return 1;
}