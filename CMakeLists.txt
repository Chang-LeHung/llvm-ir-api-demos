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
add_executable(llvm_fib ir/llvm_fib.cpp)
target_link_libraries(llvm_fib ${LLVM_LIBS})

add_executable(test_unq test/test_unq.cpp)
add_executable(vararg ir/vararg.cpp)
target_link_libraries(vararg ${LLVM_LIBS})

add_executable(call_libc ir/call_libc.cpp)
target_link_libraries(call_libc ${LLVM_LIBS})

add_executable(struct_demo ir/struct_demo.cpp)
target_link_libraries(struct_demo ${LLVM_LIBS})

add_executable(call_self ir/call_self.cpp)
target_link_libraries(call_self ${LLVM_LIBS})

add_executable(call_defined_in_cpp ir/call_defined_in_cpp.cpp)
target_link_libraries(call_defined_in_cpp ${LLVM_LIBS})
