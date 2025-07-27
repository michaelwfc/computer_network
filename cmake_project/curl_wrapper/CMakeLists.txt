find_package(CURL REQUIRED)

add_library(curl_wrapper STATIC curl_wrapper.cpp)
target_include_directories(curl_wrapper
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# curl_wrapper PRIVATE 地依赖 CURL 库 
target_link_libraries(curl_wrapper PRIVATE ${CURL_LIBRARIES})
# target_link_libraries(curl_wrapper PRIVATE `CURL::libcurl)
# 视频中的写法是直接使用 `CURL::libcurl`，这里已更新成更好的写法，
# `CURL_LIBRARIES` 是 `find_package(CURL REQUIRED)` 时设置的变量，
# 见 https://cmake.org/cmake/help/latest/module/FindCURL.html