SET(GCC_COVERAGE_COMPILE_FLAGS "-O3 -ffast-math -s")

# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.0)

# Name of the project
project(YM2609)

# Create a few variables for the folder names, so they are easier to rename in
# the future
set(EFF_DIR src/eff)

# Add all the source files needed to build the executable
add_executable(
    Project 
    src/main.cpp
    src/fmgen.h
    src/fmvgen.h
    ${EFF_DIR}/chorus.h
    ${EFF_DIR}/CRingBuffur.h
    ${EFF_DIR}/CMyFilter.h
    ${EFF_DIR}/distortion.h
    ${EFF_DIR}/eq3band.h
    ${EFF_DIR}/HPFLPF.h
    ${EFF_DIR}/IRingBuffur.h
    ${EFF_DIR}/reverb.h
    ${EFF_DIR}/ReversePhase.h)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")