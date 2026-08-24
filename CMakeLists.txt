cmake_minimum_required(VERSION 3.16)
project(TradeFlow LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_RPATH "/Users/dasrupdip/miniconda3/lib")
set(CMAKE_INSTALL_RPATH "/Users/dasrupdip/miniconda3/lib")

include(FetchContent)

find_package(Threads REQUIRED)

find_path(ASIO_INCLUDE_DIR NAMES asio.hpp PATHS /opt/homebrew/include /usr/local/include)
if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "asio.hpp not found. Install the asio package with Homebrew or package manager.")
endif()
if(NOT TARGET asio::asio)
    add_library(asio::asio INTERFACE IMPORTED)
    target_include_directories(asio::asio INTERFACE ${ASIO_INCLUDE_DIR})
    target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE)
endif()

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/libpq")

FetchContent_Declare(
    libpqxx
    GIT_REPOSITORY https://github.com/jtv/libpqxx.git
    GIT_TAG 7.9.2
)
FetchContent_MakeAvailable(libpqxx)

find_package(Crow REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB TRADEFLOW_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/api/*.cpp
    src/domain/*.cpp
    src/execution/*.cpp
    src/infrastructure/*.cpp
    src/repository/*.cpp
    src/risk/*.cpp
    src/services/*.cpp
)

add_library(tradeflow_lib STATIC ${TRADEFLOW_SOURCES})
target_include_directories(tradeflow_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(tradeflow_lib PUBLIC Threads::Threads pqxx nlohmann_json::nlohmann_json Crow::Crow OpenSSL::SSL OpenSSL::Crypto)

target_compile_definitions(tradeflow_lib PUBLIC CROW_ENABLE_SSL=0)

add_executable(tradeflow_app src/main.cpp)
target_link_libraries(tradeflow_app PRIVATE tradeflow_lib)

add_executable(tradeflow_benchmark src/benchmark.cpp)
target_link_libraries(tradeflow_benchmark PRIVATE tradeflow_lib)

enable_testing()

add_executable(test_risk_manager tests/test_risk_manager.cpp)
target_link_libraries(test_risk_manager PRIVATE tradeflow_lib GTest::gtest_main)
add_test(NAME test_risk_manager COMMAND test_risk_manager)

add_executable(test_position tests/test_position.cpp)
target_link_libraries(test_position PRIVATE tradeflow_lib GTest::gtest_main)
add_test(NAME test_position COMMAND test_position)

add_executable(test_pnl tests/test_pnl.cpp)
target_link_libraries(test_pnl PRIVATE tradeflow_lib GTest::gtest_main)
add_test(NAME test_pnl COMMAND test_pnl)

add_executable(test_order_service tests/test_order_service.cpp)
target_link_libraries(test_order_service PRIVATE tradeflow_lib GTest::gtest_main)
add_test(NAME test_order_service COMMAND test_order_service)

add_executable(test_concurrency tests/test_concurrency.cpp)
target_link_libraries(test_concurrency PRIVATE tradeflow_lib GTest::gtest_main)
add_test(NAME test_concurrency COMMAND test_concurrency)
