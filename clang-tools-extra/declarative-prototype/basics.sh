#!/bin/bash

# No Control
./bin/declarative-prototype "int main() {int x=2; x = x+3+3; int y=3; x=(x + y) - 3; y=6; x++; return 0;}"
./bin/declarative-prototype "int main() {int x=2; x = x+3+3; int y=3; int z=x+y; x=(x + y) - 3; y=6; x++; return 0;}"
./bin/declarative-prototype "int main() {int a = 2; int b = a; a = 3; b; return 0;}"
./bin/declarative-prototype "int main(){int a = 0; if (true){int b = a; a = 3;} else {2;} return 1;}"

# If Statement
./bin/declarative-prototype "int main(){int a = 0; int b = a; if (true){a = 2;} else {2;} b; return 1;}"
./bin/declarative-prototype "int main(){int a = 0; int b = a; if (true){2;} else {a=2;} b; return 1;}"
./bin/declarative-prototype "int main(){int a = 0; int b = a; if (true){2;} else if(false){a=2;} else {2;} b; return 1;}"

# For Statements
./bin/declarative-prototype "int main(){int a = 0; int b = a; for(int i=0; i<10; i++){a = 3;} b; return 1;}"
./bin/declarative-prototype "int main(){int a = 0; int b = a; for(int i=0; i<10; i++){a = i;} b; return 1;}"

# While Statements
./bin/declarative-prototype "int main(){int a = 0; int b = a; while(true){a = 3;} b; return 1;}"