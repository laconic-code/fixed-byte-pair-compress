###############
# CompressCli #
###############

add_executable(
    CompressCli
    CompressCli.cpp)

target_include_directories(
    CompressCli
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/../src/)

target_link_libraries(
    CompressCli
    PUBLIC
    Dictionary)

install(
    TARGETS CompressCli
    DESTINATION bin)

###########
# Example #
###########

add_executable(
    Example
    Example.cpp)

target_include_directories(
    Example
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/../src/)

target_link_libraries(
    Example
    PUBLIC
    Dictionary)

install(
    TARGETS Example
    DESTINATION bin)