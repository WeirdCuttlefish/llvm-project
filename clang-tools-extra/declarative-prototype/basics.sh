#!/bin/bash

./bin/declarative-prototype "int main() {int x=2; x = x+3+3; int y=3; x=(x + y) - 3; y=6; x++; return 0;}"
./bin/declarative-prototype "int main() {int x=2; x = x+3+3; int y=3; int z=x+y; x=(x + y) - 3; y=6; x++; return 0;}"
./bin/declarative-prototype "int main() {int a = 2; int b = a; a = 3; b; return 0;}"
./bin/declarative-prototype "int main(){int a = 0; if (true){int b = a; a = 3; b;} return 1;}"