# Declarative Prototype

This pass finds variables that have yet to be updated.

## How to Build and Run
First clone the pauljoo28/llvm-project repo and then build the pass. You should have CMake 2.8.6 or later installed and the Ninja build system.
```
git clone https://github.com/pauljoo28/llvm-project.git
cd llvm-project
mkdir build
cmake -G Ninja ../llvm -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" -DLLVM_BUILD_TESTS=ON
ninja declarative-prototype
```
This will take approximately an hour or two so get yourself some lunch.

When you are back, from the build file you can run
```
./bin/declarative-prototype "int main() {int a = 2; int b = a; a = 3; b; return 0;}"
```
and see the magic work!

## More Examples?
There are more examples in the `basics.sh` file that you can take from.
