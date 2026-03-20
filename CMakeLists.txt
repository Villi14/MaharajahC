cmake_minimum_required(VERSION 3.25)

project(Maharajah VERSION 0.1.0 LANGUAGES C)

set(BINARY ${PROJECT_NAME})

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/sources/*.c)
file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/headers/*.h)

# C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(${BINARY} ${SOURCES} ${HEADERS})