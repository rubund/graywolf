
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_library(GrayWolfGetDate SHARED getdate.c)

install(
	TARGETS GrayWolfGetDate
	DESTINATION ${CMAKE_INSTALL_LIBDIR}
	)

