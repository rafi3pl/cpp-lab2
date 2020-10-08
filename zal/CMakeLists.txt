cmake_minimum_required(VERSION 3.8)

project(lab1_zal)

add_executable(zal test.cpp)
target_include_directories(zal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(zal PROPERTIES
    CXX_STANDARD 17
)
