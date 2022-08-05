cmake_minimum_required(VERSION 3.13...3.22)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
      "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Vcpkg toolchain file")
endif ()

project(
    hijo
    VERSION 0.0.1
    HOMEPAGE_URL https://lesharris.com/hijo
    LANGUAGES C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 20)

# Dependencies
find_package(raylib 4.0.0 QUIET)
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
      raylib
      URL https://github.com/raysan5/raylib/archive/refs/tags/4.0.0.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif ()
endif ()

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")

find_package(fmt CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)

# Our App!
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/Hijo.cpp
    src/core/Hijo.h
    src/common/common.h
    src/core/events/EventManager.h
    src/core/events/Event.h
    src/core/events/Events.h
    src/core/input/Input.cpp
    src/core/input/Input.h
    src/core/input/Keys.h
    src/core/input/InputActions.h
    src/core/input/Mapping.h
    src/core/layers/GameLayerStack.cpp
    src/core/layers/GameLayerStack.h
    src/core/layers/GameLayer.h
    src/layers/Emu.cpp
    src/layers/Emu.h
    src/system/Gameboy.cpp
    src/system/Gameboy.h
    src/system/System.h
    src/external/imgui_extra/imgui_impl_glfw.cpp
    src/external/imgui/backends/imgui_impl_opengl3.cpp
    src/external/imgui/imgui_demo.cpp
    src/external/imgui/imgui_draw.cpp
    src/external/imgui/imgui_tables.cpp
    src/external/imgui/imgui_widgets.cpp
    src/external/imgui/imgui.cpp
    src/layers/UI.cpp
    src/layers/UI.h
    src/external/imgui_extra/imgui_memory_editor.h
    src/cartridge/Cartridge.cpp
    src/cartridge/Cartridge.h
    src/common/common.cpp
    src/cpu/Instructions.h
    src/cpu/SharpSM83.cpp
    src/cpu/SharpSM83.h
    src/cpu/Interrupts.cpp
    src/cpu/Interrupts.h
    src/display/LCD.cpp
    src/display/LCD.h
    src/display/PPU.cpp
    src/display/PPU.h
    src/cpu/Timer.cpp
    src/cpu/Timer.h
    src/cpu/DMA.cpp
    src/cpu/DMA.h
    src/cpu/Stack.cpp
    src/cpu/Stack.h src/input/Controller.cpp src/input/Controller.h src/sound/APU.cpp src/sound/APU.h src/cartridge/mappers/Mapper.h src/cartridge/mappers/ROM.cpp src/cartridge/mappers/ROM.h src/cartridge/mappers/MBC1.cpp src/cartridge/mappers/MBC1.h)

# Compile Options
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif ()

if (MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else ()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif ()

# Generated Files
configure_file("src/common/config.h.in" "src/common/config.h")

# Include directories and libraries
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src/common
    ${STB_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/external
    ${PROJECT_SOURCE_DIR}/src/external/imgui
    ${PROJECT_SOURCE_DIR}/src/external/imgui/backends)

target_link_libraries(${PROJECT_NAME} PRIVATE
    fmt::fmt
    raylib
    ZLIB::ZLIB
    spdlog::spdlog
    EnTT::EnTT)

if (APPLE)
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
  target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif ()

