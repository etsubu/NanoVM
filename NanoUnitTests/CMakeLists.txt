# CMakeList.txt : CMake project for NanoVM, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
include_directories(../NanoVM)
# Add source to this project's executable.
# add_executable (NanoUnitTests "test.cpp" "../NanoAssembler/NanoAssembler.cpp" "../NanoAssembler/NanoAssembler.h" "../NanoVM/NanoVM.cpp" "../NanoVM/NanoVM.h" "NanoDebugger.h" "Instructions.cpp" "Instructions.h" "Debugger.cpp")
add_executable (NanoUnitTests "test.cpp" "../NanoAssembler/NanoAssembler.cpp" "../NanoAssembler/NanoAssembler.h" "../NanoAssembler/Mapper.h" "../NanoAssembler/Mapper.cpp" "../NanoAssembler/types.h" "../NanoVM/NanoVM.cpp" "../NanoVM/NanoVM.h")
# TODO: Add tests and install targets if needed.

set_property(TARGET NanoUnitTests PROPERTY CXX_STANDARD 20)
set_property(TARGET NanoUnitTests PROPERTY CXX_STANDARD_REQUIRED ON)