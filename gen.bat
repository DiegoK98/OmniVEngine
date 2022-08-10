if not exist build mkdir build
cd build
cmake -S ../ -B . -G "Visual Studio 17 2022" -A x64
cd ..
