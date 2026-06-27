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
  if (ANDROID)
    add_link_options("-Wl,-z,max-page-size=16384")
  endif()
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

add_executable(compound_piece_smoke tests/compound_piece_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(compound_piece_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(maharajah_tool tools/maharajah_tool.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(maharajah_tool PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(engine_rules_smoke tests/engine_rules_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(engine_rules_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(engine_config_smoke tests/engine_config_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(engine_config_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(search_sanity_smoke tests/search_sanity_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(search_sanity_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(evaluate_safety_smoke tests/evaluate_safety_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(evaluate_safety_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(special_moves_smoke tests/special_moves_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(special_moves_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(draw_rules_smoke tests/draw_rules_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(draw_rules_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(ffi_api_smoke tests/ffi_api_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(ffi_api_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(transposition_smoke tests/transposition_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(transposition_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

add_executable(see_smoke tests/see_smoke.c ${TEST_SOURCES} ${MAHARAJAH_ENGINE_HEADERS})
target_include_directories(see_smoke PRIVATE
  ${MAHARAJAH_ENGINE_INCLUDE_DIR}
)

enable_testing()
add_test(NAME perft_smoke COMMAND perft_smoke)
add_test(NAME compound_piece_smoke COMMAND compound_piece_smoke)
add_test(NAME engine_rules_smoke COMMAND engine_rules_smoke)
add_test(NAME engine_config_smoke COMMAND engine_config_smoke)
add_test(NAME search_sanity_smoke COMMAND search_sanity_smoke)
add_test(NAME evaluate_safety_smoke COMMAND evaluate_safety_smoke)
add_test(NAME special_moves_smoke COMMAND special_moves_smoke)
add_test(NAME draw_rules_smoke COMMAND draw_rules_smoke)
add_test(NAME ffi_api_smoke COMMAND ffi_api_smoke)
add_test(NAME transposition_smoke COMMAND transposition_smoke)
add_test(NAME see_smoke COMMAND see_smoke)
add_test(NAME maharajah_tool_generate COMMAND maharajah_tool generate 1 7)
add_test(NAME maharajah_tool_selfplay COMMAND maharajah_tool selfplay 1 1 20 7)
add_test(NAME maharajah_tool_selfplay_json
  COMMAND ${PROJECT_SOURCE_DIR}/tests/maharajah_tool_selfplay_json.sh
          $<TARGET_FILE:maharajah_tool>
          ${PROJECT_BINARY_DIR}/maharajah_tool_selfplay.json)
add_test(NAME maharajah_tool_bench
  COMMAND /bin/sh ${PROJECT_SOURCE_DIR}/tests/maharajah_tool_bench_smoke.sh
          $<TARGET_FILE:maharajah_tool>)
add_test(NAME uci_smoke COMMAND ${PROJECT_SOURCE_DIR}/tests/uci_smoke.sh $<TARGET_FILE:${BINARY}>)
