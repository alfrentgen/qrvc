cmake_minimum_required(VERSION 3.5)

project("tests" C CXX)

set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(INC_DIR)
#StegaModule test
set(STEG_HDRS ${INC_DIR}/steganography.h)
set(STEG_SRCS ${SRC_DIR}/steganography.cpp)
add_executable(steg_module_test
				stega_test.cpp
				${STEG_HDRS}
				${STEG_SRCS}
                )
target_include_directories(steg_module_test PRIVATE ${INC_DIR})
#add_dependencies(steg_module_test libc)
#set_target_properties(steg_module_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
#target_link_libraries(steg_module_test )
				
add_test(NAME stega_test.cpp COMMAND stega_test.cpp WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
set_property(TARGET	steg_module_test PROPERTY FOLDER "Tests")