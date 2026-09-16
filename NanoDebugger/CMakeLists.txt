# CMakeList.txt : CMake project for NanoVM, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
include_directories(../NanoVM)
# Add source to this project's executable.
add_executable (NanoDebugger "NanoDebugger.c" "../NanoVM/NanoVM.c" "../NanoVM/NanoVM.h" "../NanoVM/NanoHeap.c" "../NanoVM/NanoHeap.h" "NanoDebugger.h" "Debugger.c")

# TODO: Add tests and install targets if needed.
