# CMakeList.txt : CMake project for NanoVM, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (NanoVM "Nano.cpp" "NanoVM.cpp" "NanoVM.h")

# TODO: Add tests and install targets if needed.