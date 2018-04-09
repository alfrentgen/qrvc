cmake_minimum_required(VERSION 3.5)

message(STATUS ${GENERATOR})

project("QR video codec" C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(USE_X86_32 "Build for x86 32 bits"	OFF)
#option(USE_ARM_V8A "Build for ARMv8-A 64"	OFF)
#option(BUILD_STATIC "Static linking"	ON)

#[[if(USE_ARM_V8A)
    #arm-linux-androideabi-gcc
    #arm-linux-androideabi-g++
    #arm-linux-androideabi-ar
    #arm-linux-androideabi-ld
    set(CMAKE_C_COMPILER /usr/bin/arm-linux-androideabi-gcc)
    set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-androideabi-g++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")
    set(USE_X86_32 OFF)
endif()]]

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fexceptions -fpermissive -static-libstdc++ -static-libgcc -O3 -mtune=generic -fPIC -fPIE")
set(CMAKE_C_FLAGS "-std=gnu11 -Wall -fexceptions -fpermissive -static-libstdc++ -static-libgcc -O3 -mtune=generic -fPIC -fPIE")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

#if(BUILD_STATIC)
    #https://stackoverflow.com/questions/35116327/when-g-static-link-pthread-cause-segmentation-fault-why
    #-static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static-libstdc++ -static-libgcc")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static-libstdc++ -static-libgcc")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc")
#endif()

if(USE_X86_32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -march=i686 -mfpmath=sse -msse -msse2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -march=i686 -mfpmath=sse -msse -msse2")
    set(TARBALL_ARCH "x86_32")
    #set(USE_ARM_V8A OFF)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(TARBALL_ARCH "x86_64")
endif()

#if(MINGW)	#otherwise it breaks down on Windows due to dynamic linking to libstdc++*.dll as it can't find _ZNKSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE3strEv.
			#The solution was found here https://github.com/dmlc/xgboost/issues/977.
    #if(NOT BUILD_STATIC)
        #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc")
    #endif()
	#message(STATUS "MINGW CMAKE_EXE_LINKER_FLAGS: " ${CMAKE_EXE_LINKER_FLAGS})
	#message(STATUS "MINGW CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
#endif()

message(STATUS "CMAKE_LINKER ${CMAKE_LINKER}")
message(STATUS "CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE}")
message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_C_FLAGS ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}")

#set(MUSL_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/libs/musl/lib")
#set(MUSL_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/musl/include")
#link_directories(${MUSL_LIB_DIR})
add_subdirectory(libs)

set(DEC_HDR
include/pch.h
include/Config.h
include/ArgsParser.h
include/Chunk.h
include/Job.h
include/Decode.h
include/inputQueue.h
include/OutputQueue.h
include/MTDecoder.h
include/utilities.h
include/utilities_c.h
include/help.h
)

set(DEC_SRC
src/ArgsParser.cpp
src/Chunk.cpp
src/Job.cpp
src/Decode.cpp
src/inputQueue.cpp
src/OutputQueue.cpp
src/MTDecoder.cpp
src/Config.cpp
)

add_executable(qrvd ${DEC_HDR} ${DEC_SRC})
target_include_directories(qrvd PUBLIC include)
target_compile_options(qrvd PRIVATE -pthread)
set_target_properties(qrvd PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(qrvd PROPERTIES LINK_FLAGS "-Wl,-Bstatic,-l:libstdc++.a,-l:libgcc.a -l:libc.a -Wl,-rpath,'$ORIGIN/libs' -Wl,--dynamic-linker=libs/libc.so")
target_link_libraries(qrvd -Wl,-Bdynamic quirc zbar_cut)


SET(MTENC_HDR
include/pch.h
include/Config.h
include/ArgsParser.h
include/Chunk.h
include/Job.h
include/Encode.h
include/inputQueue.h
include/OutputQueue.h
include/MTEncoder.h
include/utilities.h
include/utilities_c.h
include/help.h
)

SET(MTENC_SRC
src/ArgsParser.cpp
src/Chunk.cpp
src/Job.cpp
src/Encode.cpp
src/inputQueue.cpp
src/OutputQueue.cpp
src/MTEncoder.cpp
src/Config.cpp
)

add_executable(qrve ${MTENC_HDR} ${MTENC_SRC})
target_include_directories(qrve PUBLIC include)
target_compile_options(qrve PRIVATE -pthread)
set_target_properties(qrve PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(qrve PROPERTIES LINK_FLAGS "-Wl,-Bstatic,-l:libstdc++.a,-l:libgcc.a -l:libc.a -Wl,-rpath,'$ORIGIN/libs' -Wl,--dynamic-linker=libs/libc.so")
target_link_libraries(qrve -Wl,-Bdynamic qrenc)


#TESTS SECTION
set(TESTS_PATH  ${CMAKE_SOURCE_DIR}/tests)

#package making target
if(MINGW)
    set(TARBALL_NAME qrvc_win)
else()
    set(TARBALL_NAME qrvc_linux)
endif()
set(TARBALL_NAME "${TARBALL_NAME}_${TARBALL_ARCH}")
message(STATUS "TARBALL_NAME= ${TARBALL_NAME}")

add_custom_target(create_tar ALL
    COMMAND ${CMAKE_COMMAND} -E remove "-f" "${CMAKE_CURRENT_BINARY_DIR}/versions.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/versions.txt" "${CMAKE_CURRENT_BINARY_DIR}/versions.txt"
    COMMAND ${CMAKE_COMMAND} -E tar "-cfvz" "${TARBALL_NAME}.tar.gz" "$<TARGET_FILE:qrve>" "$<TARGET_FILE:qrvd>" "${CMAKE_CURRENT_BINARY_DIR}/versions.txt")
add_dependencies(create_tar qrvd qrve)
