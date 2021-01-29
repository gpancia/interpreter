list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../build/install ../..)

macro(msg)
    message(STATUS "${ARGV0}")
endmacro()

cmake_minimum_required(VERSION 3.16)

project(interpreter)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_BUILD_TYPE Debug)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "")
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "default install path" FORCE )
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_INSTALL_PREFIX})
list(INSERT CMAKE_PREFIX_PATH 0 ${CMAKE_INSTALL_PREFIX})

#include(MessageMacros)
message(STATUS "Building ${PROJECT_NAME}")
message(STATUS "Configuring paths")

message(STATUS "Module path: ${CMAKE_MODULE_PATH}")
message(STATUS "Prefix path: ${CMAKE_PREFIX_PATH}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(DATA_DIR_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(DATA_DIR_INSTALL ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/data)

message(STATUS "Data dir (in build tree): ${DATA_DIR_BUILD}")
message(STATUS "Data dir (in install tree): ${DATA_DIR_INSTALL}")

message(STATUS "Configuring source files")

#option(DATA_DIR_ENABLE "DDE" ON)
#if (DATA_DIR_ENABLE)
set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
#endif()
configure_file(
    ${SOURCE_DIR}/config.h.in
    ${SOURCE_DIR}/config.h
)

set(FILE_NAMES
    env
    evaluator
    expr
    flags
    lexer
    lexer_utils
    parser
    tk_line
)

list(TRANSFORM FILE_NAMES PREPEND "${SOURCE_DIR}/")
set(SOURCE_FILES ${FILE_NAMES})
set(HEADER_FILES ${FILE_NAMES})

list(TRANSFORM SOURCE_FILES APPEND ".c")
list(APPEND SOURCE_FILES "${SOURCE_DIR}/main.c")

list(TRANSFORM HEADER_FILES APPEND ".h")
list(APPEND HEADER_FILES "${SOURCE_DIR}/config.h")

set(EXTRA_FILES
    "${SOURCE_DIR}/config.h.in"
    "README.md"
)

#message(STATUS "${FILE_NAMES}\nsource files = ${SOURCE_FILES}\nheaders = ${HEADER_FILES}")

set_source_files_properties(${EXTRAFILES} PROPERTIES HEADER_FILE_ONLY TRUE)

message(STATUS "Configuring compiler options")

msg("${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -W -Wall -Wextra -fdiagnostics-color=always -Wfloat-equal -Wshadow -Wcast-align -Wcast-qual -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wswitch-default -Wno-cast-align -g -c")

message(STATUS "Building for ${CMAKE_SYSTEM_NAME}.")

# Linux specific
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DLINUX)
  set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


# Apple specific
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DOSX)

  # RPATH settings, see https://cmake.org/Wiki/CMake_RPATH_handling
  set(CMAKE_MACOSX_RPATH ON)

  # use, i.e. don't skip the full RPATH for the build tree
  SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

  # when building, don't use the install RPATH already
  # (but later on when installing)
  SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  # the RPATH to be used when installing, but only if it's not a system directory
  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  IF("${isSystemDir}" STREQUAL "-1")
     SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  ENDIF("${isSystemDir}" STREQUAL "-1")

endif()


# Windows specific
if (WIN32)
  add_definitions(-DWIN32)
endif()

msg("Defining target")

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${EXTRA_FILES})

msg("Configuring install target")

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

install(
    DIRECTORY data/
    DESTINATION ${DATA_DIR_INSTALL})
