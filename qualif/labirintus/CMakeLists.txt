cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2 REQUIRED system window graphics)


add_executable(glab
    src/gui.cpp
)

add_executable(lab
    src/main.cpp
)

target_include_directories(glab PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries(glab ${SFML_LIBRARIES})
