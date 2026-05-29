cmake_minimum_required(VERSION 3.10)
project(turbojpeg-android C)

set(LIBJPEG_TURBO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../libjpeg-turbo-3.1.0")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SIZEOF_SIZE_T 8)
else()
    set(SIZEOF_SIZE_T 4)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/jconfig.h.in" "${CMAKE_CURRENT_BINARY_DIR}/jconfig.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/jconfigint.h.in" "${CMAKE_CURRENT_BINARY_DIR}/jconfigint.h")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBJPEG_TURBO_ROOT}/src
    ${LIBJPEG_TURBO_ROOT}
)

# Files with NO precision sensitivity (compiled once at 8-bit)
set(JPEG_BASE_SOURCES
    ${LIBJPEG_TURBO_ROOT}/src/jaricom.c
    ${LIBJPEG_TURBO_ROOT}/src/jcapimin.c
    ${LIBJPEG_TURBO_ROOT}/src/jcarith.c
    ${LIBJPEG_TURBO_ROOT}/src/jchuff.c
    ${LIBJPEG_TURBO_ROOT}/src/jcicc.c
    ${LIBJPEG_TURBO_ROOT}/src/jcinit.c
    ${LIBJPEG_TURBO_ROOT}/src/jclhuff.c
    ${LIBJPEG_TURBO_ROOT}/src/jcmarker.c
    ${LIBJPEG_TURBO_ROOT}/src/jcmaster.c
    ${LIBJPEG_TURBO_ROOT}/src/jcomapi.c
    ${LIBJPEG_TURBO_ROOT}/src/jcparam.c
    ${LIBJPEG_TURBO_ROOT}/src/jcphuff.c
    ${LIBJPEG_TURBO_ROOT}/src/jctrans.c
    ${LIBJPEG_TURBO_ROOT}/src/jdapimin.c
    ${LIBJPEG_TURBO_ROOT}/src/jdarith.c
    ${LIBJPEG_TURBO_ROOT}/src/jdatadst.c
    ${LIBJPEG_TURBO_ROOT}/src/jdatadst-tj.c
    ${LIBJPEG_TURBO_ROOT}/src/jdatasrc.c
    ${LIBJPEG_TURBO_ROOT}/src/jdatasrc-tj.c
    ${LIBJPEG_TURBO_ROOT}/src/jdhuff.c
    ${LIBJPEG_TURBO_ROOT}/src/jdicc.c
    ${LIBJPEG_TURBO_ROOT}/src/jdinput.c
    ${LIBJPEG_TURBO_ROOT}/src/jdlhuff.c
    ${LIBJPEG_TURBO_ROOT}/src/jdmarker.c
    ${LIBJPEG_TURBO_ROOT}/src/jdmaster.c
    ${LIBJPEG_TURBO_ROOT}/src/jdphuff.c
    ${LIBJPEG_TURBO_ROOT}/src/jdtrans.c
    ${LIBJPEG_TURBO_ROOT}/src/jerror.c
    ${LIBJPEG_TURBO_ROOT}/src/jfdctflt.c
    ${LIBJPEG_TURBO_ROOT}/src/jmemmgr.c
    ${LIBJPEG_TURBO_ROOT}/src/jmemnobs.c
    ${LIBJPEG_TURBO_ROOT}/src/jpeg_nbits.c
    ${LIBJPEG_TURBO_ROOT}/src/rdbmp.c
    ${LIBJPEG_TURBO_ROOT}/src/rdppm.c
    ${LIBJPEG_TURBO_ROOT}/src/turbojpeg.c
    ${LIBJPEG_TURBO_ROOT}/src/transupp.c
    ${LIBJPEG_TURBO_ROOT}/src/wrbmp.c
    ${LIBJPEG_TURBO_ROOT}/src/wrppm.c
)

# DCT-based precision files (support 8-bit and 12-bit, NOT 16-bit)
# From original JPEG12_SOURCES
set(JPEG12_SOURCES
    ${LIBJPEG_TURBO_ROOT}/src/jccoefct.c
    ${LIBJPEG_TURBO_ROOT}/src/jcdctmgr.c
    ${LIBJPEG_TURBO_ROOT}/src/jdcoefct.c
    ${LIBJPEG_TURBO_ROOT}/src/jddctmgr.c
    ${LIBJPEG_TURBO_ROOT}/src/jdmerge.c
    ${LIBJPEG_TURBO_ROOT}/src/jfdctfst.c
    ${LIBJPEG_TURBO_ROOT}/src/jfdctint.c
    ${LIBJPEG_TURBO_ROOT}/src/jidctflt.c
    ${LIBJPEG_TURBO_ROOT}/src/jidctfst.c
    ${LIBJPEG_TURBO_ROOT}/src/jidctint.c
    ${LIBJPEG_TURBO_ROOT}/src/jidctred.c
    ${LIBJPEG_TURBO_ROOT}/src/jquant1.c
    ${LIBJPEG_TURBO_ROOT}/src/jquant2.c
)

# Non-DCT precision files (support 8-bit, 12-bit, AND 16-bit)
# From original JPEG16_SOURCES, plus BMP/PPM I/O
set(JPEG16_SOURCES
    ${LIBJPEG_TURBO_ROOT}/src/jcapistd.c
    ${LIBJPEG_TURBO_ROOT}/src/jccolor.c
    ${LIBJPEG_TURBO_ROOT}/src/jcdiffct.c
    ${LIBJPEG_TURBO_ROOT}/src/jclossls.c
    ${LIBJPEG_TURBO_ROOT}/src/jcmainct.c
    ${LIBJPEG_TURBO_ROOT}/src/jcprepct.c
    ${LIBJPEG_TURBO_ROOT}/src/jcsample.c
    ${LIBJPEG_TURBO_ROOT}/src/jdapistd.c
    ${LIBJPEG_TURBO_ROOT}/src/jdcolor.c
    ${LIBJPEG_TURBO_ROOT}/src/jddiffct.c
    ${LIBJPEG_TURBO_ROOT}/src/jdlossls.c
    ${LIBJPEG_TURBO_ROOT}/src/jdmainct.c
    ${LIBJPEG_TURBO_ROOT}/src/jdpostct.c
    ${LIBJPEG_TURBO_ROOT}/src/jdsample.c
    ${LIBJPEG_TURBO_ROOT}/src/jutils.c
)

# 12-bit object library: DCT files + non-DCT files
add_library(jpeg12-static OBJECT ${JPEG12_SOURCES} ${JPEG16_SOURCES})
set_property(TARGET jpeg12-static PROPERTY COMPILE_FLAGS "-DBITS_IN_JSAMPLE=12")

# 16-bit object library: non-DCT files only (DCT doesn't support 16-bit)
add_library(jpeg16-static OBJECT ${JPEG16_SOURCES})
set_property(TARGET jpeg16-static PROPERTY COMPILE_FLAGS "-DBITS_IN_JSAMPLE=16")

# 12/16-bit PPM I/O (BMP I/O doesn't support multi-precision)
add_library(turbojpeg12-ppm OBJECT
    ${LIBJPEG_TURBO_ROOT}/src/rdppm.c
    ${LIBJPEG_TURBO_ROOT}/src/wrppm.c
)
set_property(TARGET turbojpeg12-ppm PROPERTY COMPILE_FLAGS "-DBITS_IN_JSAMPLE=12 -DPPM_SUPPORTED")

add_library(turbojpeg16-ppm OBJECT
    ${LIBJPEG_TURBO_ROOT}/src/rdppm.c
    ${LIBJPEG_TURBO_ROOT}/src/wrppm.c
)
set_property(TARGET turbojpeg16-ppm PROPERTY COMPILE_FLAGS "-DBITS_IN_JSAMPLE=16 -DPPM_SUPPORTED")

# Main static library: base (8-bit) + 12-bit objects + 16-bit objects
add_library(turbojpeg STATIC
    ${JPEG_BASE_SOURCES}
    ${JPEG12_SOURCES}          # 8-bit DCT variants
    ${JPEG16_SOURCES}          # 8-bit non-DCT variants
    $<TARGET_OBJECTS:jpeg12-static>
    $<TARGET_OBJECTS:jpeg16-static>
    $<TARGET_OBJECTS:turbojpeg12-ppm>
    $<TARGET_OBJECTS:turbojpeg16-ppm>
)

target_include_directories(turbojpeg PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBJPEG_TURBO_ROOT}/src
    ${LIBJPEG_TURBO_ROOT}
)

target_compile_definitions(turbojpeg PRIVATE BMP_SUPPORTED PPM_SUPPORTED)
