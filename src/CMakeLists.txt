add_library(
	Dictionary STATIC
	Dictionary.c
	Decompress.c
	Compress.cpp)

target_include_directories(
	Dictionary PUBLIC
	${CMAKE_CURRENT_LIST_DIR})

install(
	TARGETS Dictionary
	DESTINATION lib)
