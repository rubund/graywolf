
add_test(
    NAME map9v3
    COMMAND which graywolf > /tmp/thispath
)
set_tests_properties(map9v3 PROPERTIES DEPENDS graywolf)

