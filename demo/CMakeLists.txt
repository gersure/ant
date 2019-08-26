# test files and exec

set(THIRD_LIBS ant)

#find_package(Boost 1.53 REQUIRED COMPONENTS system thread)
#include_directories(SYSTEM ${Boost_INCLUDE_DIR})
#list(APPEND THIRD_LIBS ${Boost_LIBRARIES} boost_serialization boost_system gtest)

list(APPEND THIRD_LIBS gtest fmt boost_system yaml-cpp pthread)


include_directories(../include)


set(SOURCE_FILES
        timer_manager_test.cc
        sstring_test.cpp
        )

function(add_test filepath)
    get_filename_component(filename ${filepath} NAME_WE)
    add_executable(${filename} ${filepath})
    target_link_libraries(${filename}  ${THIRD_LIBS})
endfunction(add_test)


foreach(filepath ${SOURCE_FILES})
    add_test(${filepath})
endforeach(filepath)
