#Linux:
#Compiller settings:
#flags:
#c++11
#options:
#-fexceptions
#-DSTATIC_IN_RELEASE=static
#-DMAJOR_VERSION=3
#-DMINOR_VERSION=9
#-DMICRO_VERSION=0
#-DVERSION="\\"3.9.0\\""
#
#Linker settings:
#link libraries:
#pthread
#
#Windows:
#Compiller settings:
#flags:
#c++11
#options:
#-fexceptions
#-DSTATIC_IN_RELEASE=static
#-DMAJOR_VERSION=3
#-DMINOR_VERSION=9
#-DMICRO_VERSION=0
#-DVERSION="\"3.9.0\""

cmake_minimum_required(VERSION 3.1)

project("QR video encoder" C CXX)

#set(TESTS_PATH  ${CMAKE_CURRENT_SOURCE_DIR}/tests)

SET(ENC_SRC ArgsParser.cpp encoder.cpp)
SET(ENC_HDR ArgsParser.h pch.h)

add_executable(qvsenc ${ENC_SRC} ${ENC_HDR})
target_link_libraries(qvsenc qrenc)
add_dependencies(qvsenc qrenc)
set_target_properties(qvsenc PROPERTIES LINKER_LANGUAGE CXX)
