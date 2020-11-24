# C++ > Rust

cmake_minimum_required (VERSION 3.8)

set(JMWC_CXX_STANDARD 20)

project("JMWC" 
	VERSION 0.0.0
	HOMEPAGE_URL "https://github.com/JMWComputer/JMWC")

include(CTest)

add_library(${PROJECT_NAME}Lib STATIC "include/JMWC.h" "source/JMWC.cpp")
set_target_properties(${PROJECT_NAME}Lib PROPERTIES CXX_STANDARD ${JMWC_CXX_STANDARD} CXX_STANDARD_REQUIRED True)
target_include_directories(${PROJECT_NAME}Lib PUBLIC "include" PRIVATE "source")

add_executable(${PROJECT_NAME} "executables/jmwc.cpp")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD ${JMWC_CXX_STANDARD} CXX_STANDARD_REQUIRED True)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}Lib)

target_compile_definitions(${PROJECT_NAME} PRIVATE SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/executables/")

enable_testing()