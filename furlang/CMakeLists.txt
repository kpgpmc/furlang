file(GLOB_RECURSE FURLANG_SRCS "src/**.cpp")
file(GLOB_RECURSE FURLANG_HDRS "include/**.hpp")
add_library(furlang STATIC ${FURLANG_SRCS} ${FURLANG_HDRS})
target_include_directories(furlang PUBLIC include/)