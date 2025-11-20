set(PROJECT_NAME Mouse-Interface)

cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(${PROJECT_NAME} C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT_NAME})
pico_add_extra_outputs(${PROJECT_NAME})

# Make sure TinyUSB can find tusb_config.h
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

include(FetchContent)
FetchContent_Declare(a2pico
        GIT_REPOSITORY https://github.com/oliverschmidt/a2pico.git
        GIT_TAG main
        )
FetchContent_MakeAvailable(a2pico)

target_sources(${PROJECT_NAME} PRIVATE
        board.c
        incbin.S
        main.c
        MouseInterfaceCard.c
        tusb_hid.c
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        a2pico
        pico_multicore
        pico_stdlib
        tinyusb_host
        )
