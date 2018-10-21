cmake_minimum_required (VERSION 2.6)

project(LeapMotion)

include_directories(include/)

link_directories(./lib/)

aux_source_directory(src/ DIR_SRCS)

add_executable(LeapMotion ${DIR_SRCS})

target_link_libraries(LeapMotion Leap)
