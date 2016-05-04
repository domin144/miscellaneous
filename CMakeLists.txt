cmake_minimum_required (VERSION 3.0)

project(drobne)

if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Wall -Werror -Wno-deprecated-register" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
    set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Wall -Werror" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" )
    # using Intel C++
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
    # using Visual Studio C++
endif()

#add_executable(plplot_playground plplot_playground.cpp)
#target_link_libraries(plplot_playground plplotcxxd)
add_executable(dynamic_template dynamic_template.cpp)
add_executable(cache_test cache_test.cpp cache.h utils.h trees_and_heaps.h)
