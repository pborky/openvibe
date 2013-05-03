
IF(UNIX)

  FIND_PATH(EMOKIT_INCLUDE_DIR 
    NAMES emokit/emokit.h 
    PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/include)

  FIND_LIBRARY(EMOKIT_LIB 
    NAMES emokit 
    PATHS ${OV_CUSTOM_DEPENDENCIES_PATH}/lib)

  IF(EMOKIT_LIB AND EMOKIT_INCLUDE_DIR)

    INCLUDE("FindThirdPartyLibusb-1.0")
    INCLUDE("FindThirdPartyMcrypt")
    INCLUDE("FindThirdPartyHIDAPI")

    MESSAGE(STATUS "  Found Emokit...")

    IF(LIBUSB_1_FOUND)
      IF(Mcrypt_FOUND)
        IF(HIDAPI_FOUND)
          MESSAGE(STATUS "    [  OK  ] Third party lib ${EMOKIT_LIB}") 
          MESSAGE(STATUS "    [  OK  ] Third party header ${EMOKIT_INCLUDE_DIR}")

          INCLUDE_DIRECTORIES(${LIBUSB_1_INCLUDE_DIRS})
          INCLUDE_DIRECTORIES(${Mcrypt_INCLUDE_DIR})
          INCLUDE_DIRECTORIES(${HIDAPI_INCLUDE_DIR})
          INCLUDE_DIRECTORIES(${EMOKIT_INCLUDE_DIR})

          LIST(APPEND LIBEPOC_REQUIRED_LIBS ${LIBUSB_1_LIBRARIES} ${Mcrypt_LIBS} ${HIDAPI_LIBS})

          SET(EMOKIT_LIBS ${EMOKIT_LIB} ${Mcrypt_LIBS} ${LIBUSB_1_LIBRARIES} ${HIDAPI_LIBS})
        
          TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${EMOKIT_LIBS} )
	      ADD_DEFINITIONS(-DTARGET_HAS_ThirdPartyEmokit)
        ELSE(HIDAPI_FOUND)
          MESSAGE(STATUS "    [FAILED] Third party lib HIDAPI not found.")
        ENDIF(HIDAPI_FOUND)
      ELSE(Mcrypt_FOUND)
        MESSAGE(STATUS "    [FAILED] Third party lib mcrypt not found.")
      ENDIF(Mcrypt_FOUND)
    ELSE(LIBUSB_1_FOUND)
    	MESSAGE(STATUS "    [FAILED] Third party lib libusb-1.0 not found.")
    ENDIF(LIBUSB_1_FOUND)
  ELSE()
	  MESSAGE(STATUS "  FAILED to find lib Emokit")
  ENDIF()
ELSE(UNIX)
  MESSAGE(STATUS "  FAILED to find lib Emokit on your platform (you should write cmake module).")
ENDIF(UNIX)

