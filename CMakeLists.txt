cmake_minimum_required(VERSION 3.14)

project(furlang VERSION 0.0.0 LANGUAGES CXX)

enable_testing()

find_package(GTest REQUIRED)

find_package(Doxygen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(furlang)
add_subdirectory(furvm)
add_subdirectory(furc)
add_subdirectory(furas)

if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
    set(DOXYGEN_SHOW_ENUM_VALUES YES)
    set(DOXYGEN_HIDE_UNDOC_NAMESPACES NO)
    set(DOXYGEN_HIDE_UNDOC_RELATIONS NO)
    set(DOXYGEN_FILE_PATTERNS "*.hpp")
    doxygen_add_docs(doxygen_doc ${CMAKE_SOURCE_DIR}/furlang/ ${CMAKE_SOURCE_DIR}/furvm/ ${CMAKE_SOURCE_DIR}/furc/
        ALL WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen")
endif()
