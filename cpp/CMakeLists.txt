cmake_minimum_required(VERSION 3.15)
project(data_processing)

# Ручной поиск библиотеки FFTW
find_path(FFTW3_INCLUDE_DIR fftw3.h)
find_library(FFTW3_LIBRARY NAMES fftw3 fftw3-3)

if (FFTW3_INCLUDE_DIR AND FFTW3_LIBRARY)
    message(STATUS "Found FFTW3: ${FFTW3_LIBRARY}")
else()
    message(FATAL_ERROR "FFTW3 not found!")
endif()

add_executable(${PROJECT_NAME} main.cpp
                               Generator.cpp
                               Correlator.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE ${FFTW3_LIBRARY})
target_include_directories(${PROJECT_NAME} PRIVATE ${FFTW3_INCLUDE_DIR})