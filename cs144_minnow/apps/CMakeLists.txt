#  Defines a macro (like a reusable function) called add_app that takes one argument: exec_name
macro(add_app exec_name)
  # Creates an executable from the file named exec_name.cc.
  add_executable("${exec_name}" "${exec_name}.cc")
  
  # Links the executable with the libraries minnow_debug and util_debug.
  # This means the generated binary will be linked against those libraries
  
  # The libraries minnow_debug and util_debug could be: 
  # Built elsewhere in your CMake project using add_library(...)
  # Precompiled .a or .so files found in your build tree or system
  target_link_libraries("${exec_name}" minnow_debug)
  target_link_libraries("${exec_name}" util_debug)
endmacro(add_app)

add_app(webget)