cmake_minimum_required(VERSION 3.24)

project(exchange_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(EXCHANGE_USE_GSL "Enable Microsoft.GSL integration." OFF)
option(EXCHANGE_FETCH_DEPS "Allow CMake to download missing dependencies." ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  message(FATAL_ERROR
    "AppleClang is not supported for this project. "
    "Configure with LLVM clang instead, for example:\n"
    "  cmake -S . -B build "
    "-DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++"
  )
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(FATAL_ERROR
    "This project is configured for Clang. "
    "Current compiler is '${CMAKE_CXX_COMPILER_ID}'."
  )
endif()

if(EXCHANGE_USE_GSL)
  find_package(Microsoft.GSL QUIET)
  if(NOT Microsoft.GSL_FOUND)
    if(NOT EXCHANGE_FETCH_DEPS)
      message(FATAL_ERROR
        "Microsoft.GSL not found. Install it (e.g. 'brew install ms-gsl') "
        "or configure with -DEXCHANGE_FETCH_DEPS=ON."
      )
    endif()

    include(FetchContent)
    FetchContent_Declare(
      ms_gsl
      GIT_REPOSITORY https://github.com/microsoft/GSL.git
      GIT_TAG v4.2.0
    )
    FetchContent_MakeAvailable(ms_gsl)
  endif()

  if(NOT TARGET Microsoft.GSL::GSL)
    message(FATAL_ERROR
      "Microsoft.GSL integration failed: target Microsoft.GSL::GSL is unavailable."
    )
  endif()
endif()

function(exchange_enable_strict_warnings target_name)
  target_compile_options(${target_name} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wshadow
    -Wconversion
    -Wfloat-conversion
    -Wsign-conversion
    -Wcast-qual
    -Wcast-align
    -Wformat=2
    -Wundef
    -Wnull-dereference
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wold-style-cast
    -Wzero-as-null-pointer-constant
    -Wdouble-promotion
    -Wimplicit-fallthrough
    -Wreturn-type
  )
endfunction()

set(EXCHANGE_PUBLIC_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EXCHANGE_PCH_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/exchange/core/pch.hpp")

add_library(exchange_core INTERFACE)
target_include_directories(exchange_core INTERFACE "${EXCHANGE_PUBLIC_INCLUDE_DIR}")
target_compile_features(exchange_core INTERFACE cxx_std_23)
if(EXCHANGE_USE_GSL)
  target_link_libraries(exchange_core INTERFACE Microsoft.GSL::GSL)
  target_compile_definitions(exchange_core INTERFACE EXCHANGE_HAS_GSL=1)
endif()

add_library(exchange_util STATIC
  src/exchange/util/scope_timer.cpp
)
target_link_libraries(exchange_util PUBLIC exchange_core)
target_compile_features(exchange_util PUBLIC cxx_std_23)
exchange_enable_strict_warnings(exchange_util)
target_precompile_headers(exchange_util PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${EXCHANGE_PCH_HEADER}>"
)

add_executable(main src/app/main.cpp)
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE exchange_util)
exchange_enable_strict_warnings(main)
target_precompile_headers(main PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:${EXCHANGE_PCH_HEADER}>"
)

include(CTest)
if(BUILD_TESTING)
  find_package(Catch2 3 QUIET)
  if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.7.0
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
  endif()

  add_executable(main_tests tests/main_tests.cpp)
  target_compile_features(main_tests PRIVATE cxx_std_23)
  target_link_libraries(main_tests PRIVATE Catch2::Catch2WithMain exchange_core)
  exchange_enable_strict_warnings(main_tests)
  target_precompile_headers(main_tests PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${EXCHANGE_PCH_HEADER}>"
  )

  include(Catch)
  catch_discover_tests(main_tests)
endif()
