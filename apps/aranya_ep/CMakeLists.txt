cmake_minimum_required(VERSION 3.20)
project(CFS_ARANYA_EP_APP C)

include(FetchContent)

# Trying to use corrosion to build rust aranya client capi here and avoid manual linking issues
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.5 # Optionally specify a commit hash, version tag or branch here
)

set(APP_SRC_FILES
  fsw/src/aranya_ep_app.c
)

# Build the cFS app
add_cfe_app(aranya_ep ${APP_SRC_FILES})

target_include_directories(aranya_ep PUBLIC fsw/inc)

# Determine the output directory used by aranya_ep and feed it to Corrosion.
# Note: Do not use generator expressions here; Corrosion evaluates these at configure time.
set(_ARANYA_EP_OUT_DIR "")
# Single-config path
get_target_property(_ARANYA_EP_RUNTIME_DIR aranya_ep RUNTIME_OUTPUT_DIRECTORY)
if(_ARANYA_EP_RUNTIME_DIR)
  set(_ARANYA_EP_OUT_DIR "${_ARANYA_EP_RUNTIME_DIR}")
endif()
# Multi-config path(s)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${cfg}" _CFG_U)
    get_target_property(_ARANYA_EP_RUNTIME_DIR_${_CFG_U} aranya_ep RUNTIME_OUTPUT_DIRECTORY_${_CFG_U})
    if(_ARANYA_EP_RUNTIME_DIR_${_CFG_U})
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG_U} "${_ARANYA_EP_RUNTIME_DIR_${_CFG_U}}")
      set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_CFG_U} "${_ARANYA_EP_RUNTIME_DIR_${_CFG_U}}")
    endif()
  endforeach()
endif()
# Fallback for single-config if the target didn't set a runtime dir
if(NOT _ARANYA_EP_OUT_DIR AND NOT CMAKE_CONFIGURATION_TYPES)
  set(_ARANYA_EP_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()
# Apply globals for Corrosion if we have a concrete dir
if(_ARANYA_EP_OUT_DIR)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_ARANYA_EP_OUT_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_ARANYA_EP_OUT_DIR}")
endif()

# Ensure Release profile so Corrosion copies from 'release' instead of 'debug' (single-config generators).
if(NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Set any global configuration variables such as `Rust_TOOLCHAIN` before this line!
FetchContent_MakeAvailable(Corrosion)

# Import targets defined in a package or workspace manifest `Cargo.toml` file
message(STATUS "Importing aranya-client-capi crate via corrosion...")
corrosion_import_crate(
  MANIFEST_PATH 
  aranya-client-capi/Cargo.toml
  # FLAGS --release   # not needed when CMAKE_BUILD_TYPE=Release
)

# Ensure the runtime linker looks in the module directory and (as a fallback) the app build dir.
# This keeps aranya_ep.so able to find libaranya_client_capi.so in both places.
set_target_properties(aranya_ep PROPERTIES
  BUILD_RPATH "\$ORIGIN;\$ORIGIN/../../../native/default_cpu1/apps/aranya_ep"
  INSTALL_RPATH "\$ORIGIN;\$ORIGIN/../../../native/default_cpu1/apps/aranya_ep"
)

# After building, also stage the Rust .so next to aranya_ep.so in build/exe/cpu1/cf.
# Adjust the staged dir if your mission layout differs.
set(_CFE_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../../../exe/cpu1/cf")
add_custom_command(TARGET aranya_ep POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_CFE_STAGE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:aranya_client_capi-shared>
          "${_CFE_STAGE_DIR}"
  COMMENT "Staging libaranya_client_capi.so to ${_CFE_STAGE_DIR}"
)

# Clean up so subsequent targets are unaffected
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${cfg}" _CFG_U)
    unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG_U})
    unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${_CFG_U})
  endforeach()
endif()

message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}") # resolves to cfs/build/native/default_cpu1/apps/aranya_ep

message(STATUS "Linking aranya_ep against aranya-client-capi crate...")
target_link_libraries(aranya_ep aranya_client_capi)

