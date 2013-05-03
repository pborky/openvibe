# - try to find HIDAPI library
# from http://www.signal11.us/oss/hidapi/
#
# Cache Variables: (probably not for direct use in your scripts)
#  HIDAPI_INCLUDE_DIR
#  HIDAPI_LIB
#
# Non-cache variables you might use in your CMakeLists.txt:
#  HIDAPI_FOUND
#  HIDAPI_INCLUDE_DIR
#  HIDAPI_LIBS
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2009-2010 Ryan Pavlik <rpavlik@iastate.edu> <abiryan@ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC
#
# Copyright Iowa State University 2009-2010.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

IF(UNIX)

    find_library(HIDAPI_LIB 
        NAMES hidapi hidapi-hidraw hidapi-libusb
        PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)

    find_path(HIDAPI_INCLUDE_DIR
	    NAMES hidapi/hidapi.h
        PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/include)
    
    IF ( HIDAPI_LIB AND HIDAPI_INCLUDE_DIR )
       SET( HIDAPI_FOUND TRUE )
       SET( HIDAPI_LIBS ${HIDAPI_LIB})
    ELSE()
       SET( HIDAPI_FOUND FALSE )
    ENDIF()

    IF (HIDAPI_FOUND)
        IF (NOT HIDAPI_FIND_QUIETLY)
            MESSAGE(STATUS "  Found HIDAPI ....")
            MESSAGE(STATUS "    [  OK  ] Third party lib ${HIDAPI_LIBS}")
            MESSAGE(STATUS "    [  OK  ] Third party header ${HIDAPI_INCLUDE_DIR}")
        ENDIF()
    ELSE(HIDAPI_FOUND)
        MESSAGE(STATUS "  FAILED to find lib HIDAPI ....")
    ENDIF(HIDAPI_FOUND)


ELSE(UNIX)
  MESSAGE(STATUS "  FAILED to find lib HIDAPI on your platform (you should write cmake module).")
ENDIF(UNIX)

