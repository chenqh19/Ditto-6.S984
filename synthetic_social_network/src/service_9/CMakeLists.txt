set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread")

add_executable(
	service_9
	service_9.cpp
	assembly.cpp
	${THRIFT_GEN_CPP_DIR}/service_9.cpp
	${THRIFT_GEN_CPP_DIR}/auto_microservices_types.cpp
)

target_include_directories(
	service_9 PRIVATE
	/usr/local/include/jaegertracing
)

target_link_libraries(
	service_9
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

install(TARGETS service_9 DESTINATION ./)
