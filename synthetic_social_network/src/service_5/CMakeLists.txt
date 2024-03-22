set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread -DMEMCACHED")

add_executable(
	service_5
	service_5.cpp
	assembly.cpp
	${TCP_SERVER_CPP_DIR}/connection/tcp_conn.cpp
	${TCP_SERVER_CPP_DIR}/webserver.cpp
	${TCP_SERVER_CPP_DIR}/config.cpp
	${TCP_SERVER_CPP_DIR}/utils.cpp
)

target_link_libraries(
	service_5
	pthread
)

install(TARGETS service_5 DESTINATION ./)
