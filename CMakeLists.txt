cmake_minimum_required(VERSION 3.25)

project(Maharajah VERSION 0.1.0 LANGUAGES C)

# C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "This project requires a 64-bit compiler.")
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(BINARY ${PROJECT_NAME})

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/sources/*.c)
file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/headers/*.h)

add_executable(${BINARY} ${SOURCES} ${HEADERS})