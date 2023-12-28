#################################
# Encode/Decode + Dicts Library #
#################################

option(FBP_DEBUG "Off")
option(FBP_REGISTER "On")
set(CACHE FBP_REGISTRATION_COUNT 1 CACHE STRING "max number of registered states")

file(
	GLOB FoundDictionaries
	LIST_DIRECTORIES false
	"Fbp_*.c")

add_library(
	Dictionary STATIC
	Fbp.c
	${FoundDictionaries})

target_include_directories(
	Dictionary PUBLIC
	${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(
	Dictionary PUBLIC
	FBP_REGISTRATION_COUNT="${FBP_REGISTRATION_COUNT}")

install(
	TARGETS Dictionary
	DESTINATION lib)
