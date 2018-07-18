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
if(USE_MUSL)
    add_dependencies(steg_module_test libc)
    set_target_properties(steg_module_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
elseif(NOT MINGW)#NATIVE
    set_target_properties(steg_module_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN'")
endif()
				
add_test(NAME steg_module_test COMMAND steg_module_test WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
set_property(TARGET	steg_module_test PROPERTY FOLDER "Tests")

#Steganography test 2
set(STEG_HDRS ${INC_DIR}/steganography.h)
set(STEG_SRCS ${SRC_DIR}/steganography.cpp)
add_executable(stegencdec_test
				stegencdec_test.cpp
				${STEG_HDRS}
				${STEG_SRCS}
                )
target_include_directories(stegencdec_test PRIVATE ${INC_DIR})
target_link_libraries(stegencdec_test decquirc deczbar qrenc)
if(USE_MUSL)
    add_dependencies(stegencdec_test libc)
    set_target_properties(stegencdec_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
elseif(NOT MINGW)#NATIVE
    set_target_properties(stegencdec_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN'")
endif()
				
add_test(NAME stegencdec_test COMMAND stegencdec_test WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
set_property(TARGET	stegencdec_test PROPERTY FOLDER "Tests")