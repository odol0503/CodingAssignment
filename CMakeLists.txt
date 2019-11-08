cmake_minimum_required (VERSION 3.0)

INCLUDE_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/ipc
	${PROJECT_SOURCE_DIR}/third_parties
	${PROJECT_BINARY_DIR}
)

add_subdirectory ("test_server")
add_subdirectory("test_client")

