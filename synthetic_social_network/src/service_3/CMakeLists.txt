set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread -DMONGODB")

add_executable(
	service_3
	service_3.cpp
	assembly.cpp
	${TCP_SERVER_CPP_DIR}/connection/tcp_conn.cpp
	${TCP_SERVER_CPP_DIR}/webserver.cpp
	${TCP_SERVER_CPP_DIR}/config.cpp
	${TCP_SERVER_CPP_DIR}/utils.cpp
)

target_link_libraries(
	service_3
	pthread
)

install(TARGETS service_3 DESTINATION ./)
