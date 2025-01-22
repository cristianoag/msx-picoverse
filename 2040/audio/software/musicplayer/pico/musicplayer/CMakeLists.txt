# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

set(PICO_EXTRAS_FETCH_FROM_GIT ON)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(musicplayer C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(lib/picomp3lib/src)

# Add executable. Default name is the project name, version 0.1
add_executable(musicplayer musicplayer.c )

pico_set_program_name(musicplayer "musicplayer")
pico_set_program_version(musicplayer "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(musicplayer 0)
pico_enable_stdio_usb(musicplayer 1)

# Add the standard library to the build
target_link_libraries(musicplayer
        pico_stdlib 
        pico_audio_i2s
        picomp3lib)

target_include_directories(musicplayer PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(musicplayer)

