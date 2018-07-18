

add_executable(getdate getdate.c)
set_target_properties(getdate PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
