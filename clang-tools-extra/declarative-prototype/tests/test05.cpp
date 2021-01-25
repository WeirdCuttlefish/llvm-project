/*
Single Function. If Control 3.
Output: 
WARNING! b is not updated! This error is at: 17:5
*/

int main(){
    int a = 0;
    int b = a;
    if (true){
        2;
    } else if(false){
        a=2;
    } else {
        2;
    } 
    b;
    return 1;
}