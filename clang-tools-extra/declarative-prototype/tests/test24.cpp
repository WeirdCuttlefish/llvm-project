/*
Global Pointer
Output:
WARNING: b from pointer c not updated!
*/

struct Dummy{

    int a = 3;
    int b = a;
    int *c = &b;

}


struct Dummy{
    int a;
    int b;
}

int main(){
    Dummy SD1 = new Dummy(1,2);
    Dummy SD2 = SD1;
    SD2.a = SD1.a;
    SD2.b = SD2.b;

    SD1.a = 5;

    SD2.a;   // Wrong
    SD2.b;   // Right

    SD2;     // Not analyzed

    return 0;
}

int main(){
    int SD1 = 1;
    int SD2 = SD1;

    SD1 = 5;

    SD2;

    return 0;
}

int main(){
    set<> SD1 = [1,2];
    int[] SD2 = SD1;

    SD1[0] = 5;

    SD2[0];    // Wrong

    return 0;
}