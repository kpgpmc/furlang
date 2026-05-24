file(GLOB_RECURSE FURC_SRCS "src/**.cpp")
file(GLOB_RECURSE FURC_HDRS "src/**.cpp")
add_executable(furc ${FURC_SRCS} ${FURC_HDRS})
target_include_directories(furc PRIVATE include/)