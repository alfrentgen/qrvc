cmake_minimum_required(VERSION 3.5)

project("QR libraries" C CXX)

add_definitions(-DZBAR_VERSION_MAJOR=0 -DZBAR_VERSION_MINOR=10)

set(ZBAR_LIB_PATH  ./zbar)

set(ZBAR_HDR
${ZBAR_LIB_PATH}/include/zbar.h
${ZBAR_LIB_PATH}/zbar/error.h
${ZBAR_LIB_PATH}/zbar/image.h
${ZBAR_LIB_PATH}/zbar/img_scanner.h
${ZBAR_LIB_PATH}/zbar/qrcode.h
${ZBAR_LIB_PATH}/zbar/qrcode/qrdec.h
${ZBAR_LIB_PATH}/zbar/qrcode/util.h
${CMAKE_CURRENT_SOURCE_DIR}/config.h
)

set(ZBAR_SRC
${ZBAR_LIB_PATH}/zbar/decoder.c
${ZBAR_LIB_PATH}/zbar/decoder/qr_finder.c
${ZBAR_LIB_PATH}/zbar/error.c
${ZBAR_LIB_PATH}/zbar/image.c
${ZBAR_LIB_PATH}/zbar/img_scanner.c
${ZBAR_LIB_PATH}/zbar/qrcode/bch15_5.c
${ZBAR_LIB_PATH}/zbar/qrcode/binarize.c
${ZBAR_LIB_PATH}/zbar/qrcode/isaac.c
${ZBAR_LIB_PATH}/zbar/qrcode/qrdec.c
${ZBAR_LIB_PATH}/zbar/qrcode/rs.c
${ZBAR_LIB_PATH}/zbar/qrcode/util.c
${ZBAR_LIB_PATH}/zbar/refcnt.c
${ZBAR_LIB_PATH}/zbar/scanner.c
${ZBAR_LIB_PATH}/zbar/symbol.c
${CMAKE_CURRENT_SOURCE_DIR}/qrdectxt_m.c #modified zbar/qrcode/qrdectxt.c
)

add_library(zbar_cut SHARED ${ZBAR_SRC} ${ZBAR_HDR})
set_target_properties(zbar_cut PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(zbar_cut PUBLIC
${ZBAR_LIB_PATH}/zbar
${ZBAR_LIB_PATH}/include
${ZBAR_LIB_PATH}/include/zbar
${CMAKE_CURRENT_SOURCE_DIR})

if(MINGW)
	target_link_libraries(zbar_cut iconv)
else()
    target_link_libraries(zbar_cut)
endif()
set_target_properties(zbar_cut PROPERTIES FOLDER "libraries")
target_compile_definitions(zbar_cut PRIVATE
ZBAR_VERSION_MAJOR=0
ZBAR_VERSION_MINOR=10
)

set(QUIRC_LIB_PATH  quirc)

set(QUIRC_HDR
${QUIRC_LIB_PATH}/lib/quirc.h
${QUIRC_LIB_PATH}/lib/quirc_internal.h
)

set(QUIRC_SRC
${QUIRC_LIB_PATH}/lib/version_db.c
${QUIRC_LIB_PATH}/lib/decode.c
${QUIRC_LIB_PATH}/lib/identify.c
${QUIRC_LIB_PATH}/lib/quirc.c
)

add_library(quirc SHARED ${QUIRC_SRC} ${QUIRC_HDR})
set_target_properties(quirc PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(quirc PUBLIC ${QUIRC_LIB_PATH}/lib)
set_target_properties(quirc PROPERTIES FOLDER "libraries")
target_compile_definitions(quirc PRIVATE QUIRC_MAX_REGIONS=65534)

set(QRENC_LIB_PATH  ./libqrencode)

SET(QRENC_HDR
${QRENC_LIB_PATH}/bitstream.h
${QRENC_LIB_PATH}/mask.h
${QRENC_LIB_PATH}/mmask.h
${QRENC_LIB_PATH}/mqrspec.h
${QRENC_LIB_PATH}/qrencode.h
${QRENC_LIB_PATH}/qrinput.h
${QRENC_LIB_PATH}/qrspec.h
${QRENC_LIB_PATH}/rsecc.h
${QRENC_LIB_PATH}/split.h
)

SET(QRENC_SRC
${QRENC_LIB_PATH}/bitstream.c
${QRENC_LIB_PATH}/mask.c
${QRENC_LIB_PATH}/mmask.c
${QRENC_LIB_PATH}/mqrspec.c
${QRENC_LIB_PATH}/qrenc.c
${QRENC_LIB_PATH}/qrencode.c
${QRENC_LIB_PATH}/qrinput.c
${QRENC_LIB_PATH}/qrspec.c
${QRENC_LIB_PATH}/rsecc.c
${QRENC_LIB_PATH}/split.c
)

add_library(qrenc SHARED ${QRENC_SRC} ${QRENC_HDR})
set_target_properties(qrenc PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(qrenc PUBLIC ${QRENC_LIB_PATH})
set_target_properties(qrenc PROPERTIES FOLDER "libraries")
target_compile_definitions(qrenc PRIVATE
STATIC_IN_RELEASE=static
MAJOR_VERSION=3
MINOR_VERSION=9
MICRO_VERSION=0
)

target_compile_definitions(qrenc PRIVATE
VERSION="3.9.0"
)

add_custom_target(libraries DEPENDS quirc zbar_cut qrenc)