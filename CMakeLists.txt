cmake_minimum_required(VERSION 3.16)
project(android_rkp_bridge LANGUAGES CXX)

add_library(rkp_policy STATIC
    src/rkp_policy.cpp
)
target_include_directories(rkp_policy PUBLIC include)
target_compile_features(rkp_policy PUBLIC cxx_std_17)

enable_testing()
add_executable(rkp_policy_test tests/rkp_policy_test.cpp)
target_link_libraries(rkp_policy_test PRIVATE rkp_policy)
add_test(NAME rkp_policy_test COMMAND rkp_policy_test)
