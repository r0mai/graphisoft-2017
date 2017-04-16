cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2 REQUIRED system window graphics)
find_package(Boost 1.58 REQUIRED system context coroutine)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_executable(labyrinth
    src/main.cpp
    src/solver.cpp
    src/FloodFill.cpp
    src/Util.cpp
    src/Grid.cpp
)


add_executable(glabyrinth
    src/FloodFill.cpp
    src/Util.cpp
    src/Gui.cpp
    src/Grid.cpp
)

add_executable(test-labyrinth
    src/test.cpp
    src/solver.cpp
    src/FloodFill.cpp
    src/Util.cpp
    src/Grid.cpp
)

add_executable(server
    src/server.cpp
    src/Grid.cpp
)


target_include_directories(glabyrinth PRIVATE ${SFML_INCLUDE_DIR})
target_link_libraries(glabyrinth ${SFML_LIBRARIES})

target_include_directories(server PRIVATE ${BOOST_INCLUDEDIR})
target_link_libraries(server ${Boost_SYSTEM_LIBRARY} ${Boost_CONTEXT_LIBRARY} ${Boost_COROUTINE_LIBRARY})
