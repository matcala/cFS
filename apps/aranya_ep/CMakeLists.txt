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

# Set any global configuration variables such as `Rust_TOOLCHAIN` before this line!
FetchContent_MakeAvailable(Corrosion)

# Import targets defined in a package or workspace manifest `Cargo.toml` file
message(STATUS "Importing aranya-client-capi crate via corrosion...")
corrosion_import_crate(
  MANIFEST_PATH 
  aranya-client-capi/Cargo.toml
  PROFILE release   # linking will fail with debug executable for some reason
)

# resolves to build/exe/cpu1/cf
set(_CFE_STAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/../../../../exe/cpu1/cf")

# Ensure the runtime linker looks in the module directory and (as a fallback) the app build dir.
# This keeps aranya_ep.so able to find libaranya_client_capi.so in both places.
# CMAKE_CURRENT_BINARY_DIR is build/fsw/apps/aranya_ep
# from readelf -d readelf -d ./build/exe/cpu1/cf/aranya_ep.so :
#   Library runpath: [/media/sf_cfs/build/native/default_cpu1/apps/aranya_ep]
message(STATUS "Setting BPATH and RPATH of aranya_ep.so to ${CMAKE_CURRENT_BINARY_DIR} and ${_CFE_STAGE_DIR}")
set_target_properties(aranya_ep PROPERTIES
  BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}"
  INSTALL_RPATH "${_CFE_STAGE_DIR}" # changed this to set runtime dir to exe/cpu1/cf
)

# After building, also stage the Rust .so next to aranya_ep.so in build/exe/cpu1/cf.
add_custom_command(TARGET aranya_ep POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${_CFE_STAGE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:aranya_client_capi-shared>  # -shared required for some reason, without it it wont link
          "${_CFE_STAGE_DIR}"
  COMMENT "Staging libaranya_client_capi.so to ${_CFE_STAGE_DIR}"
)

# Doest not work in copying lib into exe/cpu1/cf but we keep it here for reference
# install(TARGETS aranya_client_capi
#   DESTINATION "${_CFE_STAGE_DIR}"
#   COMPONENT runtime)

message(STATUS "Linking aranya_ep against aranya-client-capi crate...")
# IMPORTANT: replace - with _ in library name because rust changes the filename during build
target_link_libraries(aranya_ep aranya_client_capi)

