# NOTE: assumes FFTW_INCLUDE_DIR and FFTW_LINK_DIR are set 
# through -DFFTW_INCLUDE_DIR and -DFFTW_LINK_DIR or through environment vars
project(fftwpp)

set(MODULE fftwpp)

set(${MODULE}_SOURCES fftw++.cc)

include_directories(${MODULE} 
    ${FFTW_INCLUDE_DIR}
)

add_library(${MODULE} 
    STATIC 
    ${${MODULE}_SOURCES}
)

target_link_directories(${MODULE} 
    INTERFACE
    ${FFTW_LINK_DIR}
)

target_link_libraries(${MODULE}
    INTERFACE
    fftw3
)
