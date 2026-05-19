cmake_minimum_required(VERSION 4.2)

get_filename_component(
        APP_NAME
        ${CMAKE_SOURCE_DIR}
        NAME
)

project(${APP_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
include(FetchContent)


# --- OpenSSL ---------------------------------------------------------------
find_package(OpenSSL REQUIRED)

# --- libcurl ----------------------------------------------------------------
find_package(CURL REQUIRED)

# --- Boost Beast + Asio + SSL ----------------------------------------------
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

# --- Boost.PFR ---------------------------------------------------------------
FetchContent_Declare(pfr
        GIT_REPOSITORY https://github.com/boostorg/pfr.git
        GIT_TAG        2.2.0
)

FetchContent_MakeAvailable(pfr)

# --- libpqxx -----------------------------------------------------------------
FetchContent_Declare(libpqxx
        GIT_REPOSITORY https://github.com/jtv/libpqxx.git
        GIT_TAG        8.0.1
)

FetchContent_MakeAvailable(libpqxx)

# --- spdlog ------------------------------------------------------------------
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

# --- jwt-cpp ----------------------------------------------------------------
FetchContent_Declare(
        jwt-cpp
        GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
        GIT_TAG        v0.7.2
        GIT_SHALLOW    TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(jwt-cpp)

# --- nlohmann/json ----------------------------------------------------------
find_package(nlohmann_json REQUIRED)

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.*"
)

add_executable(${APP_NAME} ${SRC_FILES})

target_include_directories(${APP_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(${APP_NAME} PRIVATE
        Boost::system
        Boost::beast
        Boost::asio
        OpenSSL::SSL
        OpenSSL::Crypto
        jwt-cpp::jwt-cpp
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        CURL::libcurl
        Boost::pfr
        libpqxx::pqxx
)

target_compile_options(${APP_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
)

target_compile_definitions(${APP_NAME} PRIVATE JWT_DISABLE_PICOJSON)

if (UNIX)
    target_link_libraries(${APP_NAME} PRIVATE pthread)
endif()
