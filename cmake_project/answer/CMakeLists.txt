#[[
find_package 用于在系统中寻找已经安装的第三方库的头文件和库文件
的位置，并创建一个名为 CURL::libcurl 的库目标，以供链接。
#]]
# find_package(CURL REQUIRED)

#[[
创建一个可配置的变量，可以由上级 CMakeLists 或 cmake 命令指定变量值。

这里由于 APPID 是一个应该藏好、不应该放在代码里的值，所以建议在 cmake
命令中通过 -D 参数传入。
#]]
set(WOLFRAM_APPID
    ""
    CACHE STRING "WolframAlpha APPID")

if(WOLFRAM_APPID STREQUAL "")
    message(SEND_ERROR "WOLFRAM_APPID must not be empty")
endif()


#[[
message 可用于打印调试信息或错误信息，除了 STATUS
外还有 DEBUG WARNING SEND_ERROR FATAL_ERROR 等。
#]]
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")


# add_library(libanswer STATIC answer.cpp)

# #[[
# 给 libanswer 库目标添加 include 目录，PUBLIC 使
# 这个 include 目录能被外部使用者看到。

# 当链接 libanswer 库时，这里指定的 include 目录会被
# 自动添加到使用此库的 target 的 include 路径中。
# #]]
# target_include_directories(libanswer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# #[[
# 将 WOLFRAM_APPID 添加到编译 .cpp 文件时的 definition 列表，从而
# 可在 C++ 代码中使用。
# #]]
# target_compile_definitions(libanswer PRIVATE WOLFRAM_APPID="${WOLFRAM_APPID}")

# # libanswer 改成直接使用 wolfram 库提供的 API，无需关心 CURL
# target_link_libraries(libanswer PRIVATE wolfram)

#[[
为 libanswer 库链接 libcurl，这里 PRIVATE 和 PUBLIC 的区别是：
CURL::libcurl 库只会被 libanswer 看到，根级别的 main.cpp 中
无法 include curl 的头文件。
#]]
# target_link_libraries(libanswer PRIVATE CURL::libcurl)



#[[
INTERFACE 类型的 target 一般用于没有源文件的情况，比如
header-only 库，或者只是为了抽象地提供一组 target_xxx
的配置。

INTERFACE target 的后续所有 target_xxx 都必须也使用
INTERFACE，效果将会直接应用到链接此库的 target 上。

本步骤将 libanswer 从 STATIC target 改成 INTERFACE
target 不会影响 answer_app 中使用它的代码。
#]]
add_library(libanswer INTERFACE)
target_include_directories(libanswer
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(libanswer INTERFACE WOLFRAM_APPID="${WOLFRAM_APPID}")
target_link_libraries(libanswer INTERFACE wolfram)


#[[
指明 libanswer 要求 C++20。

这里和直接设置 CMAKE_CXX_STANDARD 的区别是：

    1. 设置 CMAKE_CXX_STANDARD 之后，从设置它的那一级开始
       include 的 subdirectory 都会继承这个变量，且应用于
       所有能看到这个变量的 target；而 target_compile_features
       只应用于单个 target。
    2. target_compile_features 可以指定更细粒度的 C++ 特性，
       例如 cxx_auto_type、cxx_lambda 等。
#]]
target_compile_features(libanswer INTERFACE cxx_std_20)


# 只在启用了 BUILD_TESTING 的情况下构建测试程序
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
