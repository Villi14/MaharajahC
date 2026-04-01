cmake_minimum_required(VERSION 3.25)

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

file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/*.h)

set(APP_SOURCES ${ALL_SOURCES})
set(TEST_SOURCES ${ALL_SOURCES})
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/src/cli/main.c)

add_executable(${BINARY} ${APP_SOURCES} ${HEADERS})

target_include_directories(${BINARY} PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

add_executable(perft_smoke tests/perft_smoke.c ${TEST_SOURCES} ${HEADERS})
target_include_directories(perft_smoke PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

enable_testing()
add_test(NAME perft_smoke COMMAND perft_smoke)
add_test(NAME uci_smoke COMMAND ${PROJECT_SOURCE_DIR}/tests/uci_smoke.sh $<TARGET_FILE:${BINARY}>)
