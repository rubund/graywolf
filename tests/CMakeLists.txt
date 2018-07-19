add_test(
    NAME install_in_tmp
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install_in_tmp.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
)
set_tests_properties(install_in_tmp PROPERTIES DEPENDS graywolf)
add_test(
    NAME map9v3
    COMMAND /bin/bash -c "echo jofsdjkf > /tmp/jofda"
)
set_tests_properties(map9v3 PROPERTIES DEPENDS install_in_tmp)

