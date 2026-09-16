# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)

project ("NanoVM" C)

# Default to an optimized build. Multi-config generators (Visual Studio) choose the
# configuration at build time, so only set this for single-config generators.
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if (MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra)
endif()

# Include sub-projects.
add_subdirectory ("NanoVM")
add_subdirectory ("NanoDebugger")

include( CTest )
enable_testing()
