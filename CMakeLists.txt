
cmake_minimum_required(VERSION 3.17)
project(language568)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES
	src/*.h
	src/*.cpp
)

add_executable(language568 ${SOURCE_FILES} ${SOURCES})

include_directories(C:/Users/Emmet/Programming/lib/libpng-1.6.0/include)

target_link_libraries(language568 C:/Users/Emmet/Programming/lib/libpng-1.6.0/lib/libpngstat.lib)
target_link_libraries(language568 C:/Users/Emmet/Programming/lib/libpng-1.6.0/lib/zlibstat.lib)
