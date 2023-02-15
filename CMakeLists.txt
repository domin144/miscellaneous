cmake_minimum_required (VERSION 3.0)

project(drobne)

if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    set(CMAKE_CXX_FLAGS "-pedantic -Wall -Werror -Wno-deprecated-register -Wno-sign-compare" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
    set(CMAKE_CXX_FLAGS "-pedantic -Wall -Werror -Wno-sign-compare" )
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel" )
    # using Intel C++
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
    # using Visual Studio C++
endif()

set(CMAKE_CXX_STANDARD 17)


#add_executable(plplot_playground plplot_playground.cpp)
#target_link_libraries(plplot_playground plplotcxxd)
add_executable(dynamic_template dynamic_template.cpp)
add_executable(cache_test cache_test.cpp cache.h utils.h trees_and_heaps.h)
add_executable(hp_4510s_fan_control hp_4510s_fan_control.cpp)
add_executable(pi pi.cpp)
