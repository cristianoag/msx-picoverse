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

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picoverse C CXX ASM)

set(PICO_BOARD pico)
set(FAMILY rp2040)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(picoverse 
    picoverse.c 
    fatfs_driver.c
    flash.cpp
    usb_msc_driver.c
    usb_descriptors.c
    ${PICO_SDK_PATH}/lib/tinyusb/src/tusb.c
    fatfs/source/ff.c
    fatfs/source/ffsystem.c
    fatfs/source/ffunicode.c
)

pico_set_linker_script(picoverse ${CMAKE_SOURCE_DIR}/memmap_custom.ld)
# pico_generate_pio_header(picoverse ${CMAKE_CURRENT_LIST_DIR}/latch.pio)

pico_set_program_name(picoverse "picoverse")
pico_set_program_version(picoverse "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(picoverse 0)
pico_enable_stdio_usb(picoverse 1)

# Add the standard library to the build
target_link_libraries(picoverse
        pico_stdlib pico_multicore
        pico_bootrom
        hardware_pio
        hardware_dma
        tinyusb_additions
        tinyusb_board
        tinyusb_device
        hardware_flash
        hardware_sync
       )

# Add the standard include files to the build
target_include_directories(picoverse PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# add fatfs includes
target_include_directories(picoverse PUBLIC
  fatfs/source
  SPIFTL
  ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(picoverse)

add_compile_definitions(
    FTL_DEBUG=0
    FLASH_DEBUG=0
    MSC_DRIVER_DEBUG=0
)