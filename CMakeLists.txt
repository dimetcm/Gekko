cmake_minimum_required(VERSION 3.11)

project(Gekko VERSION 0.1)

file(GLOB SOURCES
    *.h
    *.cpp
)

add_executable(main ${SOURCES})

target_compile_features(main PUBLIC cxx_std_20)