cmake_minimum_required(VERSION 3.5)

message(STATUS ${GENERATOR})

project("QR video codec" C CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(USE_X86_32 "Build for x86 32 bits" OFF)
option(USE_MUSL "Build using musl-cross-make" OFF)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fexceptions -fpermissive -static-libstdc++ -static-libgcc -O3 -mtune=generic")
set(CMAKE_C_FLAGS "-std=gnu11 -Wall -fexceptions -fpermissive -static-libstdc++ -static-libgcc -O3 -mtune=generic")
set(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
if(USE_MUSL)
    set(MUSL_BUILD_FLAGS "-pthread -fsplit-stack")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MUSL_BUILD_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MUSL_BUILD_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MUSL_BUILD_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${MUSL_BUILD_FLAGS}")
else()
    set(MINGW_BUILD_FLAGS "-static -Wl,--allow-multiple-definition")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MINGW_BUILD_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MINGW_BUILD_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MINGW_BUILD_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "-static")
endif()

if(USE_X86_32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -march=i686 -mfpmath=sse -msse -msse2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -march=i686 -mfpmath=sse -msse -msse2")
    set(TARBALL_ARCH "x86_32")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
    set(TARBALL_ARCH "x86_64")
endif()

message(STATUS "CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH}")
message(STATUS "CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_C_COMPILER ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_LINKER ${CMAKE_LINKER}")
message(STATUS "CMAKE_CXX_LINK_EXECUTABLE ${CMAKE_CXX_LINK_EXECUTABLE}")
message(STATUS "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_C_FLAGS ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}")

add_subdirectory(libs)

if(USE_MUSL)
    set(LD_MUSL_LINK "ld-musl.so")
    add_custom_target(libc ALL
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_FIND_ROOT_PATH}/lib/libc.so" "libs/libc.so"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_FIND_ROOT_PATH}/lib/libc.so" "libs/libc.so"
        COMMAND ln "-s" "./libs/libc.so" "${LD_MUSL_LINK}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

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
set_target_properties(qrvd PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(qrvd quirc zbar_cut)
if(USE_MUSL)
    set_target_properties(qrvd PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN/libs' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
    add_dependencies(qrvd libc)
endif()


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
set_target_properties(qrve PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(qrve qrenc)
if(USE_MUSL)
    set_target_properties(qrve PROPERTIES LINK_FLAGS "-Wl,-rpath,'$ORIGIN/libs' -Wl,--dynamic-linker,'${LD_MUSL_LINK}'")
    add_dependencies(qrve libc)
endif()



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

if(USE_MUSL)
    set(LIBS_TO_TAR libs/*.so)
elseif(MINGW)
    set(LIBS_TO_TAR libs/*.dll)
endif()

add_custom_target(create_tar ALL
    COMMAND ${CMAKE_COMMAND} -E remove "-f" "versions.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/versions.txt" "versions.txt"
    COMMAND ${CMAKE_COMMAND} -E tar "-cfvz" "${TARBALL_NAME}.tar.gz" "$<TARGET_FILE:qrve>" "$<TARGET_FILE:qrvd>" "versions.txt" "${LIBS_TO_TAR}" "${LD_MUSL_LINK}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
add_dependencies(create_tar qrvd qrve)
