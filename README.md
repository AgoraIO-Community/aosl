# 1. AOSL Library Introduction
The aosl library provides an abstract implementation of low-level common components including threads, IO, logging, memory, etc.
The API header file path for aosl: include/api

# 2. Porting Guide
# 2.1 HAL Layer Definition
When porting to a chip platform, you only need to focus on the platform directory, where:
platform/include/hal:     HAL layer interface definitions
platform/src/include/hal: HAL layer configuration
platform/src/${platform}: HAL layer implementation code

# 2.2 HAL Layer Implementation
Implement the interfaces defined in the header files in platform/include/hal.
Porting includes atomic, file, iomp, socket, memory, thread, time, etc.
You can refer to the porting implementation examples of x86_64-linux and esp32-s3 when porting.

# 3. Build Guide
There is a CMakeLists.txt file in the project root directory for building examples.
CMakeLists.txt defines some build parameters and variables to control script behavior. The default behavior of these parameters is to compile the aosl library for x86_64-linux. You can set them before using this CMakeLists.txt to adapt to other platforms.

There are two build artifacts for aosl:
- (1) Static library: libaosl.a
- (2) Header files: exported "api/", "hal/" header files

# 3.0 Default Build
You can execute the following commands to compile the aosl artifacts for x86_64-linux:
```
cd aosl
mkdir build
cmake ..
make
```
At this point, you can see the libaosl.a static library and the include exported header file directory in the build directory

# 3.1 Porting Build
# 3.1.1 Build Control Settings
* Build Options:  
  - (1) AOSL_DECLARE_PROJECT: (ON/OFF) Whether to build aosl as a standalone project, default ON. If you only want to include aosl as a subdirectory, you can set it to OFF.
  - (2) AOSL_COMPILE_LIBRARY: (ON/OFF) Whether to compile the libaosl.a static library, default ON. If you only want to use the variables (such as AOSL_ADD_SOURCES, AOSL_ADD_INCLUDES_PUBLIC), you can set it to OFF.
  - (3) AOSL_COMPILE_TEST   : (ON/OFF) Whether to compile the aosl_test program, default ON.
  - (4) AOSL_INSTALL_HEADERS: (ON/OFF) Whether to copy the aosl library API header files to the output directory, default ON.

* Build Variables:  
  - (1) AOSL_DIR: Root directory of the aosl library, if not set, it defaults to CMAKE_CURRENT_SOURCE_DIR
  - (2) CONFIG_PLATFORM: Name of the target platform to port, such as x86_64-linux, esp32-s3, default is x86_64-linux

There are two ways to set parameters:
- (1) Specify through cmake command parameters, for example:
```
cmake -DCONFIG_PLATFORM=esp32-s3 \
      -DAOSL_DECLARE_PROJECT=OFF \
      -DAOSL_COMPILE_LIBRARY=OFF
```
- (2) Set parameters before include, for example:
```
set(AOSL_DIR ${COMPONENT_PATH}/../../../../../..)
set(CONFIG_PLATFORM "esp32-s3")
set(AOSL_DECLARE_PROJECT OFF CACHE BOOL "Declare as Standalone Project" FORCE)
set(AOSL_COMPILE_LIBRARY OFF CACHE BOOL "Compile Library" FORCE)
set(AOSL_INSTALL_HEADERS OFF CACHE BOOL "Install Header Files" FORCE)
include(${AOSL_DIR}/CMakeLists.txt)
```

* Build Parameters:  
  CMakeLists.txt defines the following list variables for building:
  - (1) AOSL_ADD_OPTIONS:          Compilation options used during compilation
  - (2) AOSL_ADD_DEFINITIONS:      Macro definitions used during compilation
  - (3) AOSL_ADD_INCLUDES_PUBLIC   Header file directories that need to be exported externally during compilation
  - (4) AOSL_ADD_INCLUDES_PRIVATE  Internal header file directories used during compilation
  - (5) AOSL_ADD_SOURCES           Source files needed during compilation
  If you need to introduce aosl as a subdir when porting, you can use these variables for compilation and building.

# 3.1.2 Toolchain Settings
cmake/toolchain_template.cmake is a default toolchain description file. When porting, you can add modifications to introduce the corresponding platform's toolchain

# 3.1.3 Start Building
Use the platform's build project for compilation and building.

# 4. Build Example
You can refer to platform/src/esp32/idf-proj as a reference example for building aosl as a component:
```
# 1. Set build variables
set(AOSL_DIR ${COMPONENT_PATH}/../../../../../..)
set(CONFIG_PLATFORM "esp32-s3")
# 2. Set build options
set(AOSL_DECLARE_PROJECT OFF CACHE BOOL "Declare as Standalone Project" FORCE)
set(AOSL_COMPILE_LIBRARY OFF CACHE BOOL "Compile Library" FORCE)
set(AOSL_COMPILE_TEST    OFF CACHE BOOL "Compile test" FORCE)
set(AOSL_INSTALL_HEADERS OFF CACHE BOOL "Install Header Files" FORCE)

# 3. Use aosl/CmakeLists.txt as subdir
include(${AOSL_DIR}/CMakeLists.txt)

# 3. Use build parameters such as AOSL_ADD_SOURCES
idf_component_register(SRCS ${AOSL_ADD_SOURCES}
                       INCLUDE_DIRS ${AOSL_ADD_INCLUDES_PUBLIC}
                       PRIV_INCLUDE_DIRS ${AOSL_ADD_INCLUDES_PRIVATE}
                       REQUIRES spi_flash newlib lwip mbedtls freertos esp_netif)
target_compile_options(${COMPONENT_LIB} PRIVATE ${AOSL_ADD_OPTIONS} ${AOSL_ADD_DEFINITIONS})
```
