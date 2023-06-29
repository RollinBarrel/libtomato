cmake_minimum_required(VERSION 3.22)

file(GLOB LIBTOMATO_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_library(libtomato STATIC ${LIBTOMATO_SOURCES})
target_include_directories(libtomato
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if (WIN32)
    target_link_libraries(libtomato wsock32 ws2_32)
endif()

find_package(PythonInterp REQUIRED)
add_custom_command(
    OUTPUT ${SIGDEF_H}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/sigdef.py ${SIGDEF_DEF} ${SIGDEF_H}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sigdef.py ${SIGDEF_DEF}
)
add_custom_target(
    sigdef ALL
    DEPENDS ${SIGDEF_H}
    COMMENT "Generating Signal Definitions Header"
)