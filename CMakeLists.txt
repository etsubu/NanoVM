# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

project ("NanoVM" C)

# Include sub-projects.
add_subdirectory ("NanoVM")
add_subdirectory ("NanoDebugger")

include( CTest )
enable_testing()