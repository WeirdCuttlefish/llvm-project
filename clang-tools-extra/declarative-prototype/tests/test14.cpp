/*
Basic exception (just saving variables). Swapping variables is example. But there is way around it.
Output: 
UNSURE
*/

// How it should be done.
int swap1(){

    int temp;

    int a = 2;
    int b = 3;

    temp = b;
    b = a;
    a = temp;

    return b + a;
}

// How it shouldn't be done.
int swap2(){

    int a = 2;
    int b = 3;

    int temp = b;
    b = a;
    a = temp;

    return b + a;
}