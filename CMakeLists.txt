cmake_minimum_required(VERSION 3.24)
project(llvm_tutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(llvm_tutorial main.cpp)
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(llvm_tutorial ${llvm_libs})

add_executable(test01 test01.cpp)
add_executable(assign ir/assign.cpp)
target_link_libraries(assign ${llvm_libs})

add_executable(if ir/if.cpp)
target_link_libraries(if ${llvm_libs})

add_executable(complete_fib ir/complete_fib.cpp)
target_link_libraries(complete_fib ${llvm_libs})
