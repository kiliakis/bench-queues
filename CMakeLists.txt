# Requirements list
cmake_minimum_required(VERSION 2.6.0)
project(bench-queues CXX)
include(CheckCXXCompilerFlag)

# TODO is 4.8 really the minimum requirement?
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.8.0")
   message(FATAL_ERROR "ERROR: Insufficient compiler version, need at lest 4.8 to continue")
endif()

CHECK_CXX_COMPILER_FLAG(-std=c++11 COMPILER_HAS_CXX11)
CHECK_CXX_COMPILER_FLAG(-Ofast OFAST_ON)
CHECK_CXX_COMPILER_FLAG(-ffast-math FFAST_MATH_ON)

if(!COMPILER_HAS_CXX11)
	message(FATAL_ERROR "ERROR: Compiler is lacking support for c++11")
endif()


if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
endif()


set(WARNINGS "-Wall -Wno-reorder -Wno-sign-compare -Wstrict-aliasing")
set(CFLAGS "-std=c++11 -pthread -fstrict-aliasing")
# set(CFLAGS "-std=c++11 -pthread -fstrict-aliasing -g -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -g3 -fno-omit-frame-pointer") #-fsanitize=address")

if(__HASWELL__)
   set(CFLAGS "${CFLAGS} -D__HASWELL__")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
   set(VEC "-fopt-info-vec=vec_report.txt")
   set(WARNINGS "${WARNINGS} -Wno-unused-result")
   set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")
   set(CMAKE_CXX_FLAGS_RELEASE "-march=native")
elseif (CMAKE_CXX_COMPILER_ID MATCHES Intel)
   set(WARNINGS "${WARNINGS} -wd2102")
   # set(VEC "-qopt-report=5 -qopt-report-file=vec_report.txt")
   set(VEC "-qopt-report=3")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ipo")
   if(__MIC__)
      set(CFLAGS "${CFLAGS} -mmic -D__MIC__")
   endif()
else()
   message(FATAL_ERROR "ERROR: Only gcc and icc compilers are supported.")
endif()


set(CMAKE_CXX_FLAGS "${CFLAGS} ${WARNINGS} ${VEC} ${TIMING}")

if(OFAST_ON)
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
elseif(FFAST_MATH_ON)
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math")
else()
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

include_directories(include /usr/local/include/)
link_directories(/usr/lib /usr/lib64 /usr/local/lib /usr/local/lib64)


set(EXECUTABLE_FILES 
   tests/folly.cpp
   tests/folly-push.cpp
   tests/folly-pop.cpp
   tests/boost-static.cpp
   tests/boost-static-push.cpp
   tests/boost-static-pop.cpp
   tests/boost-dynamic.cpp
   tests/boost-dynamic-push.cpp
   tests/boost-dynamic-pop.cpp
   tests/circularfifo.cpp
   tests/circularfifo-push.cpp
   tests/circularfifo-pop.cpp
   )


foreach(src ${EXECUTABLE_FILES})
   string( REPLACE ".cpp" "" testname ${src})
   string( REGEX REPLACE "tests/" "" testname ${testname})
   add_executable( ${testname} ${src})
   # set_target_properties(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_PROJECT_NAME})
   # target_link_libraries( ${testname} ${LIBRARIES})
endforeach()

# file(GLOB_RECURSE EXECUTABLE_SOURCES "tests/*.cpp")
# set(EXECUTABLE_FILES ${EXECUTABLE_SOURCES})
# foreach (src ${EXECUTABLE_FILES})
#     file(RELATIVE_PATH testfile ${CMAKE_CURRENT_SOURCE_DIR} ${src})
#     string(REPLACE ".cpp" "" testname ${testfile})
#     # string(REPLACE "demos/" "" testname ${testname})
#     string(REGEX REPLACE "tests/.*/" "" testname ${testname})
#     add_executable(${testname} ${src})
#     set_target_properties(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY tests)
#     target_link_libraries(${testname} ${LIBRARIES})
# endforeach ()

