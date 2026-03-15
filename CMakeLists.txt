cmake_minimum_required(VERSION 3.25)

project(Maharajah C)

set(BINARY ${PROJECT_NAME})

file(GLOB SOURCES *.c)
file(GLOB HEADERS *.h)

# C standard
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(${BINARY} ${SOURCES} ${HEADERS})
