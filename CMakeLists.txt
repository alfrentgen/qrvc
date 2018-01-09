cmake_minimum_required(VERSION 3.5)

message(STATUS ${GENERATOR})

project("QR video codec" C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#option(USE_ZBAR "Enable zbar library usage."	ON)
#option(USE_QUIRC "Enable quirc library usage."	ON)
option(USE_X86_32 "Build for x86 32 bits"	OFF)
option(BUILD_STATIC "Static linking"	ON)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fexceptions -fpermissive -pthread -O3")

if(BUILD_STATIC)
    #https://stackoverflow.com/questions/35116327/when-g-static-link-pthread-cause-segmentation-fault-why
    #-static -pthread -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static-libstdc++ -static-libgcc")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static-libstdc++ -static-libgcc")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc")
endif()

if(USE_X86_32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
endif()

if(MINGW)	#otherwise it breaks down on Windows due to dynamic linking to libstdc++*.dll as it can't find _ZNKSt7__cxx1115basic_stringbufIcSt11char_traitsIcESaIcEE3strEv.
			#The solution was found here https://github.com/dmlc/xgboost/issues/977.
    if(NOT BUILD_STATIC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libstdc++ -static-libgcc")
    endif()
	message(STATUS "MINGW CMAKE_EXE_LINKER_FLAGS: " ${CMAKE_EXE_LINKER_FLAGS})
endif()

message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_C_FLAGS ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}")

add_subdirectory(libs)

set(DEC_HDR
include/pch.h
include/ArgsParser.h
include/Chunk.h
include/Job.h
include/Decode.h
include/inputQueue.h
include/OutputQueue.h
include/MTDecoder.h
include/utilities.h
include/utilities_c.h
)

set(DEC_SRC
src/ArgsParser.cpp
src/Chunk.cpp
src/Job.cpp
src/Decode.cpp
src/inputQueue.cpp
src/OutputQueue.cpp
src/MTDecoder.cpp
)

add_executable(qvsdec ${DEC_HDR} ${DEC_SRC})
target_link_libraries(qvsdec quirc zbar_cut)
add_dependencies(qvsdec quirc zbar_cut)
set_target_properties(qvsdec PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(qvsdec PUBLIC include)

SET(MTENC_HDR
include/pch.h
include/ArgsParser.h
include/Chunk.h
include/Job.h
include/Encode.h
include/inputQueue.h
include/OutputQueue.h
include/MTEncoder.h
include/utilities.h
include/utilities_c.h
)

SET(MTENC_SRC
src/ArgsParser.cpp
src/Chunk.cpp
src/Job.cpp
src/Encode.cpp
src/inputQueue.cpp
src/OutputQueue.cpp
src/MTEncoder.cpp
)

add_executable(mtqvsenc ${MTENC_HDR} ${MTENC_SRC})
target_link_libraries(mtqvsenc qrenc)
add_dependencies(mtqvsenc qrenc)
set_target_properties(mtqvsenc PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(mtqvsenc PUBLIC include)

#TESTS SECTION
set(TESTS_PATH  ${CMAKE_SOURCE_DIR}/tests)
