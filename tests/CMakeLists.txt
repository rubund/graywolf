add_test(
    NAME install_in_tmp
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install_in_tmp.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
)
set_tests_properties(install_in_tmp PROPERTIES DEPENDS graywolf)

add_test(
    NAME map9v3
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/runtest.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} map9v3
)
set_tests_properties(map9v3 PROPERTIES DEPENDS install_in_tmp)

add_test(
    NAME map9v3-twsc
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/twsc/runtest.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} map9v3
)
set_tests_properties(map9v3-twsc PROPERTIES DEPENDS install_in_tmp)

add_test(
    NAME map9v3-mincut
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mincut/runtest.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} map9v3
)
set_tests_properties(map9v3-mincut PROPERTIES DEPENDS install_in_tmp)

add_test(
    NAME map9v3-twmc
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/twmc/runtest.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} map9v3
)
set_tests_properties(map9v3-twmc PROPERTIES DEPENDS install_in_tmp)

add_test(
    NAME map9v3_other_seed
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/runtest.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} map9v3_other_seed
)
set_tests_properties(map9v3_other_seed PROPERTIES DEPENDS install_in_tmp)

