# Declarative Checker

### Build instructions
Run the following command from the llvm-project directory. Takes about an hour to build.
```
mkdir build
cd build
cmake -GNinja -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_BUILD_TYPE=X86 ../llvm -DCMAKE_BUILD_TYPE=Release
ninja
```

### Modify Include Path
I was running into errors where if I ran `./bin/clang++ test.cpp`, I would not be able to find some headers like iostream. The fix for me was by setting the `CPLUS_INCLUDE_PATH` to my system's libraries. For me this was done through the following command.
```
CPLUS_INCLUDE_PATH=/Library/Developer/CommandLineTools/usr/bin/../include/c++/v1:/Library/Developer/CommandLineTools/usr/lib/clang/12.0.0/include:/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include
```
You can probably find your own system libraries by running `clang++ test.cpp -v` where `clang++` is your default clang and that will show you the proper include directories.


### How to execute
We use the scan-build command to execute. It takes in the build command as input.
```
cd build
./bin/scan-build ./bin/clang++ test.cpp
```
To run on cmake project, refer to this blog https://baptiste-wicht.com/posts/2014/04/install-use-clang-static-analyzer-cmake.html.
