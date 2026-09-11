file(GLOB_RECURSE FURLANG_SRCS "${CMAKE_SOURCE_DIR}/src/furlang/**.cpp")
file(GLOB_RECURSE FURLANG_HDRS "${CMAKE_SOURCE_DIR}/include/furlang/**.hpp")
add_library(furlang STATIC ${FURLANG_SRCS} ${FURLANG_HDRS})
target_include_directories(furlang PUBLIC ${CMAKE_SOURCE_DIR}/include/)
