cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===============================Read me first==================================
# Available configurable variables for AOSL:
#   CONFIG_PLATFORM: Target platform (default: linux)
#     Available platforms: linux, esp32-s3, etc.
#     Usage: Set before including CMakeLists.txt or use -DCONFIG_PLATFORM=<platform>
#
#   AOSL_DIR: Library root directory (default: ${CMAKE_CURRENT_SOURCE_DIR})
#     Usage: Set before including CMakeLists.txt or use -DAOSL_DIR=<path>
#
#   AOSL_DECLARE_PROJECT: Whether declare as standalone project (default: ON)
#
#   CMAKE_TOOLCHAIN_FILE: Toolchain file path
#                         (default: use aosl/cmake/toolchain_template.cmake)
# ===============================Read me first==================================

# Project control options
option(AOSL_DECLARE_PROJECT "Whether declare as Standalone Project" ON)

# Library root directory configuration
# Set library root directory (can be overridden by setting AOSL_DIR or using -DAOSL_DIR=...)
if(NOT DEFINED AOSL_DIR)
    set(AOSL_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Platform configuration
# Specify target platform (can be overridden by setting CONFIG_PLATFORM or using -DCONFIG_PLATFORM=...)
if (NOT DEFINED CONFIG_PLATFORM)
    set(CONFIG_PLATFORM "linux")
endif()
message(STATUS "CONFIG_PLATFORM:${CONFIG_PLATFORM}")
# Check if platform directory exists
if(NOT EXISTS "${AOSL_DIR}/platform/src/${CONFIG_PLATFORM}")
    message(FATAL_ERROR "Platform '${CONFIG_PLATFORM}' not found in ${AOSL_DIR}/platform/src/")
endif()

# If declared as a standalone project, define project information
if(AOSL_DECLARE_PROJECT)
    # Set toolchain file
    if(NOT CMAKE_TOOLCHAIN_FILE)
        include(${AOSL_DIR}/cmake/toolchain_template.cmake)
    endif()

    project(aosl
            VERSION 1.0.0
            DESCRIPTION "Advanced Operating System Layer"
            HOMEPAGE_URL "https://github.com/AgoraIO-Community/aosl"
            LANGUAGES C ASM)

    # Set default build type to Release for performance optimization
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    endif()

    # Set C standard to C99 for compatibility
    set(CMAKE_C_STANDARD 99)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    message(STATUS "Declared aosl as standalone project: ${PROJECT_NAME} ${PROJECT_VERSION}")
else()
    message(STATUS "Included aosl as subproject")
endif()

# Initialize build variables
set(AOSL_ADD_OPTIONS -Wno-unused-variable -Wno-unused-function)
set(AOSL_ADD_DEFINITIONS)
set(AOSL_ADD_INCLUDES_PUBLIC)
set(AOSL_ADD_INCLUDES_PRIVATE)
set(AOSL_ADD_SOURCES)

# Include build script
include(${AOSL_DIR}/cmake/build.cmake)
