###########################################################
#
# SAMPLE_APP platform build setup
#
# This file is evaluated as part of the "prepare" stage
# and can be used to set up prerequisites for the build,
# such as generating header files
#
###########################################################

# The list of header files that control the SAMPLE_APP configuration
set(ARANYA_EP_APP_PLATFORM_CONFIG_FILE_LIST
  aranya_ep_internal_cfg.h
  aranya_ep_perfids.h
  aranya_ep_msgids.h
  aranya_ep_topicids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(ARANYA_EP_APP_CFGFILE ${ARANYA_EP_APP_PLATFORM_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${ARANYA_EP_APP_CFGFILE}" NAME_WE)
  if (DEFINED ARANYA_EP_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${ARANYA_EP_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/config/default_${ARANYA_EP_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${ARANYA_EP_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
