

add_custom_command(OUTPUT show_flows COMMAND sed 's;TWFLOWDIR;${CMAKE_INSTALL_PREFIX}/lib/graywolf;' show_flows.in > show_flows)
add_custom_target(run ALL DEPENDS show_flows)
install(FILES show_flows DESTINATION lib/graywolf/bin PERMISSIONS OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE OWNER_READ GROUP_READ WORLD_READ OWNER_WRITE)
install(FILES splt_file.a DESTINATION lib/graywolf/bin)
