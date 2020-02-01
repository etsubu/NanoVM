# CMakeList.txt : CMake project for NanoVM, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (NanoAssembler "Nano.cpp" "NanoAssembler.cpp" "Mapper.cpp" "Mapper.h" "NanoAssembler.h" "Types.h")

# TODO: Add tests and install targets if needed.
