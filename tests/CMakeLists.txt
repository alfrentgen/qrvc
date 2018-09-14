cmake_minimum_required(VERSION 3.5)

project("tests" C CXX)

set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(INC_DIR)

#Wavelet test
message(STATUS "TESTS_PATH: ${TESTS_PATH}")
add_executable(wavelet_test
				${INC_DIR}/wavelet.h
				${SRC_DIR}/wavelet.cpp
				wavelet_test.cpp
                )
target_include_directories(wavelet_test PRIVATE ${INC_DIR} ${TESTS_PATH})
#target_link_libraries(wavelet_test decquirc deczbar qrenc)
if(USE_MUSL)
    add_dependencies(wavelet_test libc)
    set_target_properties(wavelet_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
elseif(NOT MINGW)#NATIVE
    set_target_properties(wavelet_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN'")
endif()
				
add_test(NAME wavelet_test COMMAND wavelet_test WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
set_property(TARGET	wavelet_test PROPERTY FOLDER "Tests")

#DCT test
message(STATUS "TESTS_PATH: ${TESTS_PATH}")
add_executable(dct_test
				${TESTS_PATH}/naive-dct.h
				${TESTS_PATH}/naive-dct.c
				${INC_DIR}/dct.h
				dct_test.cpp
                )
target_include_directories(dct_test PRIVATE ${INC_DIR} ${TESTS_PATH})
target_link_libraries(dct_test decquirc deczbar qrenc)
if(USE_MUSL)
    add_dependencies(dct_test libc)
    set_target_properties(dct_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
elseif(NOT MINGW)#NATIVE
    set_target_properties(dct_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN'")
endif()
				
add_test(NAME dct_test COMMAND dct_test WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
set_property(TARGET	dct_test PROPERTY FOLDER "Tests")

set(STEG_HDRS ${INC_DIR}/steganography.h)
set(STEG_SRCS ${SRC_DIR}/steganography.cpp)
#StegaModule test
add_executable(steg_module_test
				stega_test.cpp
				${STEG_HDRS}
				${STEG_SRCS}
                )
target_include_directories(steg_module_test PRIVATE ${INC_DIR})
target_link_libraries(steg_module_test decquirc deczbar qrenc)
if(USE_MUSL)
    add_dependencies(steg_module_test libc)
    set_target_properties(steg_module_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
elseif(NOT MINGW)#NATIVE
    set_target_properties(steg_module_test PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN'")
endif()
				
add_test(NAME steg_module_test COMMAND steg_module_test WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>)
set_property(TARGET	steg_module_test PROPERTY FOLDER "Tests")

#Steganography test 2
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
