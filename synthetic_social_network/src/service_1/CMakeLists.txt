set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread")

add_executable(
	service_1
	service_1.cpp
	assembly.cpp
	${THRIFT_GEN_CPP_DIR}/service_1.cpp
	# ${THRIFT_GEN_CPP_DIR}/service_2.cpp
	${THRIFT_GEN_CPP_DIR}/service_3.cpp
	${THRIFT_GEN_CPP_DIR}/service_4.cpp
	${THRIFT_GEN_CPP_DIR}/auto_microservices_types.cpp
)

target_include_directories(
	service_1 PRIVATE
	/usr/local/include/jaegertracing
)

target_link_libraries(
	service_1
	${THRIFT_LIB}
	${CMAKE_THREAD_LIBS_INIT}
	${Boost_LIBRARIES}
	nlohmann_json::nlohmann_json
	OpenSSL::SSL
	/usr/local/lib/libthrift.so
	/usr/local/lib/libthriftnb.so
	jaegertracing
	Boost::log
	Boost::log_setup
	/usr/local/lib/libjaegertracing.so
	pthread
)

install(TARGETS service_1 DESTINATION ./)
