file(GLOB_RECURSE DISFURAS_SRCS "${CMAKE_SOURCE_DIR}/src/disfuras/**.cpp")
add_executable(disfuras ${DISFURAS_SRCS})
target_include_directories(disfuras PRIVATE ${CMAKE_SOURCE_DIR}/include/)
target_link_libraries(disfuras PUBLIC furlang libfurc libfurvm)
