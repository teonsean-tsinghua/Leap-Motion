cmake_minimum_required (VERSION 2.6)

project(LeapMotion)

include_directories(include/)

find_library(LIBLEAP_PATH Leap lib/)

IF(NOT LIBLEAP_PATH)

MESSAGE(FATAL_ERROR "libleap not found")

ENDIF(NOT LIBLEAP_PATH)

aux_source_directory(src/ DIR_SRCS)

add_executable(LeapMotion ${DIR_SRCS})

target_link_libraries(LeapMotion ${LIBLEAP_PATH})
