##########
# encode #
##########

add_executable(
    fbp_encode
    fbp_encode.c)

target_include_directories(
    fbp_encode
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/)

target_link_libraries(
    fbp_encode
    PUBLIC
    fbp)

install(
    TARGETS fbp_encode
    DESTINATION bin)

##########
# decode #
##########

add_executable(
    fbp_decode
    fbp_decode.c)

target_include_directories(
    fbp_decode
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/)

target_link_libraries(
    fbp_decode
    PUBLIC
    Dictionary)

install(
    TARGETS fbp_decode
    DESTINATION bin)
