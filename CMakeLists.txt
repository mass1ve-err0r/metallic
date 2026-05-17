cmake_minimum_required(VERSION 4.2)
project(metallic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
include(FetchContent)

# --- Boost Beast + Asio -----------------------------------------------------
set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES system beast asio)

FetchContent_Declare(
        Boost
        GIT_REPOSITORY https://github.com/boostorg/boost.git
        GIT_TAG        boost-1.90.0
        GIT_SHALLOW    TRUE
        GIT_SUBMODULES_RECURSE TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(Boost)

# --- spdlog -----------------------------------------------------------------
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE_HO OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS_HO OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.16.0
        GIT_SHALLOW    TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(spdlog)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.*"
)

add_executable(metallic ${SRC_FILES})

target_include_directories(metallic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(metallic PRIVATE
        Boost::system
        Boost::beast
        Boost::asio
        spdlog::spdlog
)

target_compile_options(metallic PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

if (UNIX)
    target_link_libraries(metallic PRIVATE pthread)
endif()
