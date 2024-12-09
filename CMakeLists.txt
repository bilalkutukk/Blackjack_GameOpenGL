cmake_minimum_required(VERSION 3.10)

project(BlackjackGame)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find all source and header files in the project directory
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.c"  "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_LIST_DIR}/inc/*.h")
file(GLOB_RECURSE HEADERS2 "${CMAKE_CURRENT_LIST_DIR}/src/*.h")

# Include directories
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)

# Add executable
add_executable(BlackjackGame ${SOURCES} ${HEADERS} ${HEADERS2})

# Group headers and sources for Visual Studio Solution Explorer
source_group("Header Files" FILES ${HEADERS})
source_group("Source Files" FILES ${SOURCES} ${HEADERS2})

# Link GLFW
target_link_libraries(BlackjackGame PRIVATE ${CMAKE_SOURCE_DIR}/libs/glfw3.lib ${CMAKE_SOURCE_DIR}/libs/glm.lib ${CMAKE_SOURCE_DIR}/libs/freetyped.lib)

# Set the working directory to the same as CMakeLists.txt
set_target_properties(BlackjackGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
