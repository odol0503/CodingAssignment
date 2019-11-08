cmake_minimum_required (VERSION 3.0)

SET( SRC_FILES 
	test_server.cpp
	../ipc/server.hpp
	../ipc/common.hpp
	)
	
# Add source to this project's executable.
add_executable (test_server ${SRC_FILES} )