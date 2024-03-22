set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -masm=intel -lpthread")

add_executable(
	service_10
	service_10.cpp
	assembly.cpp
	${THRIFT_GEN_CPP_DIR}/service_10.cpp
	${THRIFT_GEN_CPP_DIR}/auto_microservices_types.cpp
)

target_include_directories(
	service_10 PRIVATE
	/usr/local/include/jaegertracing
)

target_link_libraries(
	service_10
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

install(TARGETS service_10 DESTINATION ./)
