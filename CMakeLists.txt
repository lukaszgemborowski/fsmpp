cmake_minimum_required (VERSION 3.2)
project (fsmpp)
set (CMAKE_CXX_STANDARD 11)

set(include-dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(test-dir ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(meta-dir ${CMAKE_CURRENT_SOURCE_DIR}/meta/include)
set(test-sources
	${test-dir}/basic.cc
	${test-dir}/simplest.cc
)

add_executable(tests ${test-sources})
target_include_directories(tests PRIVATE ${include-dir} ${tests} ${meta-dir})

include(CTest)
add_test(
	NAME "unit tests"
	COMMAND tests)
