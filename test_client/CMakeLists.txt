cmake_minimum_required (VERSION 3.0)

SET( SRC_FILES 
	test_client.cpp 
	../ipc/client.hpp
	../ipc/common.hpp
)

# Add source to this project's executable.
add_executable (test_client ${SRC_FILES} )