# Purpose: Collects all .cc source files in the current directory (e.g., src/) 
# and puts them in the variable LIB_SOURCES.
file(GLOB LIB_SOURCES "*.cc")

# Purpose: Defines a static library called minnow_debug.
# STATIC means it builds libminnow_debug.a from the source files in LIB_SOURCES.
# This is probably your default debug version (e.g., with -g and no optimization), used in local testing or development.
add_library(minnow_debug STATIC ${LIB_SOURCES})

# minnow_sanitized is another static library built from the same .cc files.
# EXCLUDE_FROM_ALL means CMake won't build this by default unless you explicitly ask it (e.g., make minnow_sanitized).
add_library(minnow_sanitized EXCLUDE_FROM_ALL STATIC ${LIB_SOURCES})

# target_compile_options(...) sets compiler flags (like -fsanitize=address) for this library.
# ${SANITIZING_FLAGS} is likely defined elsewhere and might include things like:
# set(SANITIZING_FLAGS "-fsanitize=address -fno-omit-frame-pointer")
target_compile_options(minnow_sanitized PUBLIC ${SANITIZING_FLAGS})

# Another variant of the same library, but optimized for performance.
# -O2 enables common optimizations for faster code.
# Again, EXCLUDE_FROM_ALL means it won’t be built unless explicitly requested.
add_library(minnow_optimized EXCLUDE_FROM_ALL STATIC ${LIB_SOURCES})
target_compile_options(minnow_optimized PUBLIC "-O2")


# Why Multiple Variants?
# This lets developers easily switch between:
# minnow_debug: normal debugging
# minnow_sanitized: memory safety testing
# minnow_optimized: performance benchmarking or release builds