# -*- mode: CMAKE; -*-

cmake_minimum_required(VERSION 2.8)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release
    CACHE string
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
    FORCE
  )
endif ()

if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"
      OR CMAKE_BUILD_TYPE STREQUAL "Release"
      OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
      OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel"
      OR CMAKE_BUILD_TYPE STREQUAL "None"))
  message(FATAL_ERROR "expecting CMAKE_BUILD_TYPE: None Debug Release RelWithDebInfo MinSizeRel, got ${CMAKE_BUILD_TYPE}.")
endif ()

project(curlpp CXX) 

# need c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_COMPILER_IS_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
  set(CMAKE_COMPILER_IS_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
  set(CMAKE_COMPILER_IS_INTEL 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_COMPILER_IS_INTEL 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_COMPILER_IS_GNUCC 1)
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
endif ()

#########################################################################################
# Root folders
#########################################################################################

set(ProjectRootDir ${${PROJECT_NAME}_SOURCE_DIR}/)
message(STATUS "Using Sources From: ${ProjectRootDir}")

set(ProjectBinaryDir ${${PROJECT_NAME}_BINARY_DIR}/)
message(STATUS "Putting all major output in: ${ProjectBinaryDir}")

# Source Subfolders
set(SrcFolder ${ProjectRootDir}src/)
message(STATUS "Inputs of type .cpp at: ${SrcFolder}")

set(IncludeFolder ${ProjectRootDir}include/)
message(STATUS "Inputs of type .h and .hpp at: ${IncludeFolder}")

set(ExamplesFolder ${ProjectRootDir}examples/)
message(STATUS "Inputs to be built as standalone examples are at: ${ExamplesFolder}")

#########################################################################################
# Look for dependencies
#########################################################################################

message(STATUS "Looking for CURL")

include(FindCURL)
find_package(CURL REQUIRED)

if (${CURL_FOUND})
  message(STATUS "Found CURL version: ${CURL_VERSION_STRING}")
  message(STATUS "Using CURL include dir(s): ${CURL_INCLUDE_DIRS}")
  message(STATUS "Using CURL lib(s): ${CURL_LIBRARIES}")
else ()
  message(FATAL_ERROR "Could not find CURL")
endif ()

#########################################################################################
# Create Library
#########################################################################################

include_directories( 
  ${IncludeFolder}
  ${CURL_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}_OBJS OBJECT
  ${SrcFolder}utilspp/PrivateMembers.cpp
  ${SrcFolder}utilspp/LifetimeLibrary.cpp
  ${SrcFolder}curlpp/Form.cpp
  ${SrcFolder}curlpp/Exception.cpp
  ${SrcFolder}curlpp/Options.cpp
  ${SrcFolder}curlpp/Option.cpp
  ${SrcFolder}curlpp/Info.cpp
  ${SrcFolder}curlpp/Multi.cpp
  ${SrcFolder}curlpp/OptionBase.cpp
  ${SrcFolder}curlpp/internal/SList.cpp
  ${SrcFolder}curlpp/internal/OptionSetter.cpp
  ${SrcFolder}curlpp/internal/OptionList.cpp
  ${SrcFolder}curlpp/internal/CurlHandle.cpp
  ${SrcFolder}curlpp/Easy.cpp
  ${SrcFolder}curlpp/cURLpp.cpp
)

add_library(${PROJECT_NAME} STATIC
  $<TARGET_OBJECTS:${PROJECT_NAME}_OBJS>
)

target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})

#########################################################################################
# Create Examples
#########################################################################################

set(ExamplesFileList
  ${ExamplesFolder}example00.cpp
  ${ExamplesFolder}example01.cpp
  ${ExamplesFolder}example02.cpp
  ${ExamplesFolder}example03.cpp
  ${ExamplesFolder}example04.cpp
  ${ExamplesFolder}example05.cpp
  ${ExamplesFolder}example06.cpp
  ${ExamplesFolder}example07.cpp
  ${ExamplesFolder}example08.cpp
  ${ExamplesFolder}example09.cpp
  ${ExamplesFolder}example10.cpp
  ${ExamplesFolder}example11.cpp
  ${ExamplesFolder}example12.cpp
  ${ExamplesFolder}example13.cpp
  ${ExamplesFolder}example14.cpp
  ${ExamplesFolder}example16.cpp
  ${ExamplesFolder}example15.cpp
  ${ExamplesFolder}example17.cpp
  ${ExamplesFolder}example18.cpp
  ${ExamplesFolder}example19.cpp
  ${ExamplesFolder}example20.cpp
  ${ExamplesFolder}example21.cpp
  ${ExamplesFolder}example22.cpp
  ${ExamplesFolder}example23.cpp
  ${ExamplesFolder}example24.cpp
)

# Create a meta target to create examples
add_custom_target(curlpp_examples
  COMMENT
    A target that requires all the examples.
)

foreach (Example ${ExamplesFileList})
  get_filename_component(ExampleName ${Example} NAME_WE)

  add_executable(${ExampleName} EXCLUDE_FROM_ALL ${Example})
  target_link_libraries(${ExampleName} ${PROJECT_NAME})

  add_dependencies(curlpp_examples ${ExampleName})
endforeach ()

#########################################################################################
# Create Examples
#########################################################################################

install(FILES
  include/curlpp/Easy.hpp
  include/curlpp/Easy.inl
  include/curlpp/Exception.hpp
  include/curlpp/Form.hpp
  include/curlpp/Info.hpp
  include/curlpp/Info.inl
  include/curlpp/Infos.hpp
  include/curlpp/Multi.hpp
  include/curlpp/Option.hpp
  include/curlpp/Option.inl
  include/curlpp/OptionBase.hpp
  include/curlpp/Options.hpp
  include/curlpp/Types.hpp
  include/curlpp/cURLpp.hpp
  DESTINATION include/curlpp
)

install(FILES
  include/curlpp/internal/CurlHandle.hpp
  include/curlpp/internal/CurlHandle.inl
  include/curlpp/internal/OptionContainer.hpp
  include/curlpp/internal/OptionContainer.inl
  include/curlpp/internal/OptionContainerType.hpp
  include/curlpp/internal/OptionList.hpp
  include/curlpp/internal/OptionSetter.hpp
  include/curlpp/internal/OptionSetter.inl
  include/curlpp/internal/SList.hpp
  include/curlpp/internal/buildconfig.h
  DESTINATION include/curlpp/internal
)

install(FILES
  include/utilspp/EmptyType.hpp
  include/utilspp/Functors.hpp
  include/utilspp/NonCopyable.hpp
  include/utilspp/NullType.hpp
  include/utilspp/Singleton.hpp
  include/utilspp/SmartPtr.hpp
  include/utilspp/ThreadingFactoryMutex.hpp
  include/utilspp/ThreadingSingle.hpp
  include/utilspp/ThreadingSingle.inl
  include/utilspp/TypeList.hpp
  include/utilspp/TypeTrait.hpp
  include/utilspp/clone_ptr.hpp
  DESTINATION include/utilspp
)

install(FILES
  include/utilspp/functor/Binder.hpp
  include/utilspp/functor/Functor.hpp
  include/utilspp/functor/FunctorHandler.hpp
  include/utilspp/functor/FunctorImpl.hpp
  include/utilspp/functor/MemFunHandler.hpp
  DESTINATION include/utilspp/functor
)

install(FILES
  include/utilspp/singleton/CreationStatic.hpp
  include/utilspp/singleton/CreationStatic.inl
  include/utilspp/singleton/CreationUsingNew.hpp
  include/utilspp/singleton/CreationUsingNew.inl
  include/utilspp/singleton/LifetimeDefault.hpp
  include/utilspp/singleton/LifetimeDefault.inl
  include/utilspp/singleton/LifetimeLibrary.hpp
  include/utilspp/singleton/LifetimeLibrary.inl
  include/utilspp/singleton/LifetimeWithLongevity.hpp
  include/utilspp/singleton/LifetimeWithLongevity.inl
  include/utilspp/singleton/PrivateMembers.hpp
  include/utilspp/singleton/PrivateMembers.inl
  include/utilspp/singleton/SingletonHolder.hpp
  include/utilspp/singleton/SingletonHolder.inl
  include/utilspp/singleton/ThreadingFactoryMutex.inl
  DESTINATION include/utilspp/singleton
)
