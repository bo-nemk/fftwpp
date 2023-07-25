# NOTE: assumes FFTW_INCLUDE_DIR and FFTW_LINK_DIR are set 
# through -DFFTW_INCLUDE_DIR and -DFFTW_LINK_DIR or through environment vars
project(fftwpp)

set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib") # Library files destination
set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include") # Header files destination

set(MODULE fftwpp)

add_library(${MODULE} 
    STATIC 
    fftw++.cc
)

include_directories(${MODULE} 
    ${FFTW_INCLUDE_DIR}
    ./include/
)

target_link_directories(${MODULE} 
    INTERFACE
    ${FFTW_LINK_DIR}
)

target_link_libraries(${MODULE}
    INTERFACE
    fftw3
)

install(TARGETS ${MODULE} DESTINATION "${INSTALL_LIB_DIR}")
install(DIRECTORY ./include DESTINATION "${INSTALL_LIB_DIR}")
