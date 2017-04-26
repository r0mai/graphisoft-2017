cmake_minimum_required(VERSION 3.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED system context coroutine program_options)

add_executable(labyrinth
    src/main.cpp
    src/Client.cpp
    src/FloodFill.cpp
    src/Util.cpp
    src/Grid.cpp
    src/EagerTaxicab.cpp
    src/UpwindSailer.cpp
    src/SuperFill.cpp
    src/Bounds.cpp
    src/InputParser.cpp
)


target_include_directories(labyrinth PRIVATE ${BOOST_INCLUDE_DIRS})
target_link_libraries(labyrinth ${Boost_PROGRAM_OPTIONS_LIBRARY})
