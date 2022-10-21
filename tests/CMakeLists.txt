cmake_minimum_required (VERSION 3.8)
project ("CameraWXGL")
set(CMAKE_CXX_STANDARD 17)

add_executable(test main.cpp)

set(NetworkLibPath ... CACHE PATH "path to Network lib")
set(ProjectPath ... CACHE PATH "path to tested project")

include_directories(${NetworkLibPath}/inc)
include_directories(${ProjectPath}/inc)

target_link_libraries(test "Ws2_32.lib")

install(TARGETS test DESTINATION ${CMAKE_INSTAL_PREFIX})

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()