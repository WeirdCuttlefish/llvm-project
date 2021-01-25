/*
Single Function. While Control 1.
Output: 
WARNING! b is not updated! This error is at: 13:5
*/

int main(){
    int a = 0;
    int b = a;
    while(true){
        a = 3;
    }
    b;
    return 1;
}