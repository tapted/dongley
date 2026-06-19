set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE DEPS_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/espbase/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src/*.cpp
)
file(GLOB_RECURSE MAIN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp
)

set(PROJECT_SOURCES ${MAIN_SRCS} ${DEPS_SRCS})

idf_component_register(SRCS ${PROJECT_SOURCES} INCLUDE_DIRS ".")

target_include_directories(${COMPONENT_LIB} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/espbase/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/halpp/src
)