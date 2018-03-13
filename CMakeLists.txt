cmake_minimum_required(VERSION 3.5)
project(FSeedCB)

SET(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES

        rio.c
        rio.h
        http.c
        http.h
        signal.c
        signal.h
        main.c
        )

add_executable(FSeedCB ${SOURCE_FILES})


TARGET_LINK_LIBRARIES( FSeedCB pthread)
