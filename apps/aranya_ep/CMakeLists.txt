cmake_minimum_required(VERSION 3.20)
project(CFS_ARANYA_EP_APP C)

include(FetchContent)

# 1) Ensure both rustup (in ~/.cargo/bin) and toolchain/bin are visible:
# set(ENV{PATH} "/home/jstar/.cargo/bin:/home/jstar/.rustup/toolchains/1.85-aarch64-unknown-linux-gnu/bin:$ENV{PATH}")

# # Trying to use corrosion to build rust aranya client capi here and avoid linking issues
# FetchContent_Declare(
#     Corrosion
#     GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
#     GIT_TAG v0.5 # Optionally specify a commit hash, version tag or branch here
# )

# # Ensure RPATH is defined before target creation so INSTALL_RPATH is inherited
# set(_CF_RUNTIME_DIR "${CMAKE_INSTALL_PREFIX}/${TGTSYS_${SYSVAR}}/${INSTALL_SUBDIR}")
# set(CMAKE_INSTALL_RPATH "${_CF_RUNTIME_DIR}")

# Local/public includes
# include_directories(
#   mission_inc
#   platform_inc
#   fsw/src
#   include         # aranya-client.h placed here
#   ${hwlib_MISSION_DIR}/fsw/public_inc
# )

set(APP_SRC_FILES
  fsw/src/aranya_ep_app.c
)

# aux_source_directory(src APP_SRC_FILES)


# Build the cFS app
add_cfe_app(aranya_ep ${APP_SRC_FILES})

target_include_directories(aranya_ep PUBLIC fsw/inc)


# Also force both build-tree and install-tree RPATH explicitly
# set_target_properties(aranya_ep PROPERTIES
#   BUILD_RPATH "${_CF_RUNTIME_DIR}"
#   INSTALL_RPATH "${_CF_RUNTIME_DIR}"
# )

# --- Link against the Aranya C API at build time ---
# If you have headers only, this is optional (symbols resolve at runtime).
# But it's fine to link now if the lib is present.
# set(ARANYA_SO ${CMAKE_CURRENT_SOURCE_DIR}/lib/libaranya_client.so)

# message(STATUS "Enabling Corrosion to build/link aranya-client-capi...")
# # Trying to match this with aranya capi rust toolchain version
# set(Rust_COMPILER "/home/jstar/.rustup/toolchains/1.85-aarch64-unknown-linux-gnu/bin/rustc" CACHE FILEPATH "")
# set(Rust_CARGO    "/home/jstar/.rustup/toolchains/1.85-aarch64-unknown-linux-gnu/bin/cargo" CACHE FILEPATH "")

# message(STATUS "ENV PATH = [$ENV{PATH}]")
# execute_process(COMMAND /usr/bin/env which rustup RESULT_VARIABLE rv1 OUTPUT_VARIABLE out1 ERROR_VARIABLE err1)
# message(STATUS "which rustup -> rv=${rv1} out=${out1} err=${err1}")
# execute_process(COMMAND /usr/bin/env which rustc  RESULT_VARIABLE rv2 OUTPUT_VARIABLE out2 ERROR_VARIABLE err2)
# message(STATUS "which rustc  -> rv=${rv2} out=${out2} err=${err2}")
# message(STATUS "ENV PATH = [$ENV{PATH}]")
# execute_process(COMMAND /bin/sh -lc "command -v rustup || true" OUTPUT_VARIABLE R1)
# execute_process(COMMAND /bin/sh -lc "command -v rustc  || true" OUTPUT_VARIABLE R2)
# message(STATUS "command -v rustup: ${R1}")
# message(STATUS "command -v rustc:  ${R2}")

# Set any global configuration variables such as `Rust_TOOLCHAIN` before this line!
# FetchContent_MakeAvailable(Corrosion)

# Import targets defined in a package or workspace manifest `Cargo.toml` file
# message(STATUS "Importing aranya-client-capi crate via corrosion...")
# corrosion_import_crate(MANIFEST_PATH aranya-client-capi/Cargo.toml)

# message(STATUS "Linking aranya_ep against aranya-client-capi crate...")
# target_link_libraries(aranya_ep aranya-client-capi)

# if(EXISTS "${ARANYA_SO}")
# 	target_link_libraries(aranya_ep ${ARANYA_SO})

# 	# --- Copy the Aranya .so into the same dir as aranya_ep.so after build ---
# 	# cFE stages modules into build/exe/<cpu>/cf; $<TARGET_FILE_DIR:aranya_ep> resolves there.
# 	add_custom_command(TARGET aranya_ep POST_BUILD
# 	  COMMAND ${CMAKE_COMMAND} -E copy_if_different
# 	          "${ARANYA_SO}"
# 	          "$<TARGET_FILE_DIR:aranya_ep>/libaranya_client.so"
# 	  COMMAND ${CMAKE_COMMAND} -E make_directory "${_CF_RUNTIME_DIR}"
# 	  COMMAND ${CMAKE_COMMAND} -E copy_if_different
# 	          "${ARANYA_SO}"
# 	          "${_CF_RUNTIME_DIR}/libaranya_client.so"
# 	  COMMENT "Staging libaranya_client.so (build dir and runtime ${_CF_RUNTIME_DIR})"
# 	)
# else()
# 	message(WARNING "Aranya client library not found at ${ARANYA_SO}; building aranya_ep without direct linkage.")
# endif()
