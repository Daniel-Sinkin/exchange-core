cmake_minimum_required(VERSION 3.24)

project(exchange_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

add_executable(main src/main.cpp)
target_compile_features(main PRIVATE cxx_std_23)

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
  target_link_libraries(main_tests PRIVATE Catch2::Catch2WithMain)

  include(Catch)
  catch_discover_tests(main_tests)
endif()
