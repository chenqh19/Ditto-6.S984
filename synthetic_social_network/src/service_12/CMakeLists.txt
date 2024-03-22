set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread -DMONGODB")

add_executable(
	service_12
	service_12.cpp
	assembly.cpp
	${TCP_SERVER_CPP_DIR}/connection/tcp_conn.cpp
	${TCP_SERVER_CPP_DIR}/webserver.cpp
	${TCP_SERVER_CPP_DIR}/config.cpp
	${TCP_SERVER_CPP_DIR}/utils.cpp
)

target_link_libraries(
	service_12
	pthread
)

install(TARGETS service_12 DESTINATION ./)
