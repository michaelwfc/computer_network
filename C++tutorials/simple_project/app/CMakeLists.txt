# Create an executable named 'myapp' from main.cc
add_executable(myapp main.cc)

# Link the executable against the library 'mylib'
# myapp: The target being built (defined earlier by add_executable).
# mylib: The library target to link against (defined in lib/CMakeLists.txt by add_library).

# When you compile main.cc, the compiler creates an object file (main.o). 
# This object file contains a call to Hello::say_hello, but it doesn't contain the actual code for that function. 
# The code for Hello::say_hello lives in hello.cc, which was compiled into the static library libmylib.a.

target_link_libraries(myapp mylib)