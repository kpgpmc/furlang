cmake_minimum_required(VERSION 3.14)

project(furlang VERSION 0.0.0 LANGUAGES CXX)

enable_testing()

find_package(GTest REQUIRED)

find_package(Doxygen)

add_subdirectory(furlang)
add_subdirectory(furc)

if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/docs)
    doxygen_add_docs(doxygen_doc ${CMAKE_SOURCE_DIR}/furc/ ${CMAKE_SOURCE_DIR}/furlang/
        ALL WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen")
endif()