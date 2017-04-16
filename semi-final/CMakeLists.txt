cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2 REQUIRED system window graphics)

add_executable(labyrinth
    src/main.cpp
    src/solver.cpp
)


add_executable(glabyrinth
    src/gui.cpp
)

target_include_directories(glabyrinth PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries(glabyrinth ${SFML_LIBRARIES})
