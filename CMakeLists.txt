cmake_minimum_required(VERSION 3.22.1)

project(Maharajah VERSION 0.1.0 LANGUAGES C)

# C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -O1 -g)
#     add_link_options(-fsanitize=address,undefined)
# endif()

if (MSVC)
  add_compile_options(/W4)
  add_compile_options(/D_CRT_NONSTDC_NO_WARNINGS)
  add_compile_options(/D_CRT_SECURE_NO_WARNINGS)
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

include(${CMAKE_CURRENT_SOURCE_DIR}/MaharajahEngine.cmake)

set(APP_SOURCES ${MAHARAJAH_ENGINE_SOURCES})
set(TEST_SOURCES ${MAHARAJAH_ENGINE_SOURCES})
list(REMOVE_ITEM TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/cli/main.c)

option(MAH_BUILD_FFI_SHARED "Build shared FFI library" ON)
option(MAH_BUILD_FFI_STATIC "Build static FFI library" ON)

add_executable(${BINARY} ${APP_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})

target_include_directories(${BINARY} PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

if (MAH_BUILD_FFI_SHARED)
  add_library(maharajah_ffi SHARED ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
  target_include_directories(maharajah_ffi PRIVATE
    ${MAHARAJAH_ENGINE_INCLUDE_DIR}
  )
  target_compile_definitions(maharajah_ffi PRIVATE MAH_FFI_BUILD)
  set_target_properties(maharajah_ffi PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

if (MAH_BUILD_FFI_STATIC)
  add_library(maharajah_ffi_static STATIC ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
  target_include_directories(maharajah_ffi_static PRIVATE
    ${MAHARAJAH_ENGINE_INCLUDE_DIR}
  )
  target_compile_definitions(maharajah_ffi_static PRIVATE MAH_FFI_BUILD)
  set_target_properties(maharajah_ffi_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

add_executable(perft_smoke tests/perft_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(perft_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

enable_testing()
add_test(NAME perft_smoke COMMAND perft_smoke)
add_test(NAME uci_smoke COMMAND ${PROJECT_SOURCE_DIR}/tests/uci_smoke.sh $<TARGET_FILE:${BINARY}>)
