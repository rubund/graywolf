

add_custom_command(OUTPUT show_flows COMMAND sed 's;TWFLOWDIR;${CMAKE_INSTALL_PREFIX}/lib/graywolf;' ${CMAKE_SOURCE_DIR}/script/show_flows.in > ${CMAKE_BINARY_DIR}/script/show_flows)
add_custom_target(run ALL DEPENDS ${CMAKE_BINARY_DIR}/script/show_flows)
install(FILES ${CMAKE_BINARY_DIR}/script/show_flows DESTINATION lib/graywolf/bin PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)
install(FILES splt_file.a DESTINATION lib/graywolf/bin)
