cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2 REQUIRED system window graphics)
find_package(Boost 1.58 REQUIRED system context coroutine program_options)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


add_library(liblabyrinth STATIC
    src/Client.cpp
    src/FloodFill.cpp
    src/Util.cpp
    src/Grid.cpp
    src/EagerTaxicab.cpp
    src/Bounds.cpp
)

add_executable(labyrinth
    src/main.cpp
)

add_executable(glabyrinth
    src/Gui.cpp
    src/Hsv2rgb.cpp
)

add_executable(test-labyrinth
    src/test.cpp
)

add_executable(server
    src/server.cpp
    src/Grid.cpp
    src/Util.cpp
    src/server/Command.cpp
)


target_include_directories(glabyrinth PRIVATE
    ${SFML_INCLUDE_DIR}
    ${BOOST_INCLUDEDIR}
)
target_link_libraries(glabyrinth
    liblabyrinth
    ${SFML_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_link_libraries(test-labyrinth
    liblabyrinth
)

target_include_directories(labyrinth PRIVATE ${BOOST_INCLUDEDIR})
target_link_libraries(labyrinth
    liblabyrinth
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

target_include_directories(server PRIVATE ${BOOST_INCLUDEDIR})
target_link_libraries(server
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_CONTEXT_LIBRARY}
    ${Boost_COROUTINE_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)
