set(IC_USE_CXX ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/isocline)

file(GLOB_RECURSE FURDB_SRCS "src/**.cpp")
file(GLOB_RECURSE FURDB_HDRS "include/**.hpp")
add_executable(furdb ${FURDB_SRCS} ${FURDB_HDRS})
target_include_directories(furdb PRIVATE include/)
target_link_libraries(furdb PUBLIC furlang libfurc libfurvm isocline)
