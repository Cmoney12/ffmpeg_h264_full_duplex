cmake_minimum_required(VERSION 3.20)
project(openh264_example)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} )

FIND_PATH(OPENH264_INCLUDE_DIR
        NAMES wels/codec_api.h
        PATHS /usr/local/include/
        /usr/include/)
MARK_AS_ADVANCED(OPENH264_INCLUDE_DIR)
FIND_LIBRARY(OPENH264_LIBRARY
        NAMES openh264
        PATHS ${LIBOPENH264DIR}/lib/
        /usr/lib/arm-linux-gnueabihf/
        /usr/lib/arm-linux-gnueabi/
        /usr/lib/x86_64-linux-gnu/
        /usr/local/lib64/
        /usr/lib64/
        /usr/lib/)
MARK_AS_ADVANCED(OPENH264_LIBRARY)

###########################################################################
IF (OPENH264_INCLUDE_DIR
        AND OPENH264_LIBRARY)
    SET(OPENH264_FOUND 1)
    SET(OPENH264_LIBRARIES ${OPENH264_LIBRARY})
    SET(OPENH264_INCLUDE_DIRS ${OPENH264_INCLUDE_DIR})
ENDIF()

MARK_AS_ADVANCED(OPENH264_LIBRARIES)
MARK_AS_ADVANCED(OPENH264_INCLUDE_DIRS)

IF (OPENH264_FOUND)
    MESSAGE(STATUS "Found openh264: ${OPENH264_INCLUDE_DIRS}, ${OPENH264_LIBRARIES}")
ELSE ()
    MESSAGE(STATUS "Could not find openh264")
ENDIF()

add_executable(openh264_example main.cpp)

target_link_libraries(openh264_example ${OPENCV_LIBS} ${OPENH264_LIBRARy})