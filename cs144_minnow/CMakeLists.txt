# cmake_minimum_required(VERSION 3.24.2)
cmake_minimum_required(VERSION 3.22.1)


# The project(minnow CXX) command explicitly specifies that this project uses C++ as its primary language, 
# while project(minnow) would let CMake auto-detect languages.
project(minnow CXX)

if(${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
  set(CMAKE_MAKE_PROGRAM "${PROJECT_SOURCE_DIR}/scripts/make-parallel.sh" CACHE STRING "" FORCE)
endif()

if(${PROJECT_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Minnow must be built outside its source directory, e.g. `cmake -B build`.")
endif()

# CMake processes one script file at a time. When it encounters include(...), it executes the contents of another CMake file, 
# just like copy-pasting it into the main CMakeLists.txt.

# Loads configuration related to build types, like Debug, Release, RelWithDebInfo, etc.
include(etc/build_type.cmake)

# Sets compiler flags like -Wall, -O2, -std=c++17, etc.
include(etc/cflags.cmake)
# May define custom commands or scripts to scan directories, generate files, or process templates.
include(etc/scanners.cmake)

# Adds unit/integration tests using add_test(...) or enable_testing().
# Can define test executables and link them with test frameworks (like gtest, Catch2, etc.)
include(etc/tests.cmake)

#[[
Purpose: Tells the compiler where to look for header files (.h, .hpp).
What it does: Adds ${PROJECT_SOURCE_DIR}/src to the include path for #include directives.
So in your .cc files, you can do: #include "my_header.h"
instead of needing to write:      #include "src/my_header.h"
#]]
include_directories("${PROJECT_SOURCE_DIR}/util")
include_directories("${PROJECT_SOURCE_DIR}/src")

#[[
Purpose: Tells CMake to enter another directory and process its CMakeLists.txt, which may:
define new libraries,
add new source files,
set compiler flags,
define tests,
etc.
What it does:
Executes the CMakeLists.txt in util/
Usually used when a project has submodules or components (e.g. util, net, core)
Will actually compile any targets (libraries, executables) defined there.
#]]
add_subdirectory("${PROJECT_SOURCE_DIR}/util")
add_subdirectory("${PROJECT_SOURCE_DIR}/tests")

# add_subdirectory("${PROJECT_SOURCE_DIR}/src")
# add_subdirectory("${PROJECT_SOURCE_DIR}/apps")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



# Set C++ standard before project declaration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#[[


  cmake -S . -B build
  cmake --build build

Meaning:
-S . means source directory is current directory (.)
-B build means build directory is build/

#]]