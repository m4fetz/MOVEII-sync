INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_MOVEII moveii)

FIND_PATH(
    MOVEII_INCLUDE_DIRS
    NAMES moveii/api.h
    HINTS $ENV{MOVEII_DIR}/include
        ${PC_MOVEII_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    MOVEII_LIBRARIES
    NAMES gnuradio-moveii
    HINTS $ENV{MOVEII_DIR}/lib
        ${PC_MOVEII_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MOVEII DEFAULT_MSG MOVEII_LIBRARIES MOVEII_INCLUDE_DIRS)
MARK_AS_ADVANCED(MOVEII_LIBRARIES MOVEII_INCLUDE_DIRS)

