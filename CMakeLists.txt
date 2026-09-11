# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

project ("NanoVM" C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# Include sub-projects.
add_subdirectory ("NanoVM")
add_subdirectory ("NanoDebugger")

include( CTest )
enable_testing()
