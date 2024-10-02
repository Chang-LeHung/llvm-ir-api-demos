cmake_minimum_required(VERSION 3.24)
project(llvm_tutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(llvm_tutorial main.cpp)
llvm_map_components_to_libnames(llvm_libs
        support
        core
        irreader
        orcjit
        executionengine
        native
        nativecodegen
        aarch64)
# 使用 llvm-config 获取库
execute_process(COMMAND llvm-config --libs all
        OUTPUT_VARIABLE LLVM_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE)

target_link_libraries(llvm_tutorial ${llvm_libs})

add_executable(test01 test01.cpp)
add_executable(assign ir/assign.cpp)
target_link_libraries(assign ${LLVM_LIBS})

add_executable(if ir/if.cpp)
target_link_libraries(if ${LLVM_LIBS})

add_executable(complete_fib ir/complete_fib.cpp)
target_link_libraries(complete_fib ${LLVM_LIBS})

add_executable(add_jit ir/add_jit.cpp)
target_link_libraries(add_jit ${LLVM_LIBS})

add_executable(move_test test/move_test.cpp)
add_executable(test_template test/test_template.cpp)