cmake_minimum_required (VERSION 3.0)
project (breakasm CXX)

add_definitions (-D_LINUX)

# Main application

set(CMAKE_BUILD_TYPE Release)

add_executable (breakasm 
	main.cpp 
	asm.cpp
	asmops.cpp
	asmexpr.cpp
)
