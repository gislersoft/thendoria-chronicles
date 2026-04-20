cmake_minimum_required(VERSION 3.16)
project(ThendoriaPort LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_executable(thendoria_port
    src/main.cpp
    src/Renderer320x200.cpp
    src/GraphCompat.cpp
    src/SpriteCompat.cpp
    src/FontCompat.cpp
    src/SimpleFont.cpp
    src/MapData.cpp
    src/TileSetCompat.cpp
    src/IntroScreen.cpp
)

target_link_libraries(thendoria_port PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)

if(MSVC)
    target_compile_options(thendoria_port PRIVATE /W4)
else()
    target_compile_options(thendoria_port PRIVATE -Wall -Wextra -Wpedantic)
endif()
