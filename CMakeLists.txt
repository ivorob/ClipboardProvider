CMAKE_MINIMUM_REQUIRED (VERSION 3.0)
PROJECT (ClipboardProvider)

SET (SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (SOURCES
        ${SRC_DIR}/main.cpp
        ${SRC_DIR}/ClipboardProvider.cpp
    )

SET (HEADERS
        ${SRC_DIR}/ClipboardProvider.h
    )

SET (CXX_STANDARD 11)

ADD_EXECUTABLE (ClipboardProvider ${SOURCES})
TARGET_LINK_LIBRARIES (ClipboardProvider X11)
