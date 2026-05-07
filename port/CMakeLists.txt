cmake_minimum_required(VERSION 3.16)
project(ThendoriaPort LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer CONFIG QUIET)
if(NOT SDL2_mixer_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2_MIXER QUIET SDL2_mixer)
        if(SDL2_MIXER_FOUND)
            set(SDL2_mixer_FOUND TRUE)
            if(NOT TARGET SDL2_mixer::SDL2_mixer)
                add_library(SDL2_mixer::SDL2_mixer INTERFACE IMPORTED)
                target_include_directories(SDL2_mixer::SDL2_mixer INTERFACE ${SDL2_MIXER_INCLUDE_DIRS})
                target_link_libraries(SDL2_mixer::SDL2_mixer INTERFACE ${SDL2_MIXER_LIBRARIES})
                target_link_directories(SDL2_mixer::SDL2_mixer INTERFACE ${SDL2_MIXER_LIBRARY_DIRS})
            endif()
        endif()
    endif()
endif()
message(STATUS "SDL2_mixer found: ${SDL2_mixer_FOUND}")

add_executable(thendoria_port
    src/main.cpp
    src/InputHandler.cpp
    src/Renderer320x200.cpp
    src/GraphCompat.cpp
    src/SpriteCompat.cpp
    src/FontCompat.cpp
    src/SimpleFont.cpp
    src/MapData.cpp
    src/TileSetCompat.cpp
    src/IntroScreen.cpp
    src/BattleMode.cpp
)

add_executable(thendoria_map_editor
    src/MapEditor.cpp
    src/MapData.cpp
)

target_link_libraries(thendoria_port PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)
target_link_libraries(thendoria_map_editor PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)

if(SDL2_mixer_FOUND)
    target_link_libraries(thendoria_port PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
    target_compile_definitions(thendoria_port PRIVATE THENDORIA_HAVE_SDL_MIXER=1)
endif()

if(MSVC)
    target_compile_options(thendoria_port PRIVATE /W4)
    target_compile_options(thendoria_map_editor PRIVATE /W4)
else()
    target_compile_options(thendoria_port PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(thendoria_map_editor PRIVATE -Wall -Wextra -Wpedantic)
endif()
