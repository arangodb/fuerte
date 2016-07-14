# -*- mode: CMAKE; -*-

project(curlpp) 
cmake_minimum_required(VERSION 2.8)

# need c++11
set(CMAKE_CXX_STANDARD 11)

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
add_custom_target(examples
  COMMENT
    A target that requires all the examples.
)

foreach (Example ${ExamplesFileList})
  get_filename_component(ExampleName ${Example} NAME_WE)

  add_executable(${ExampleName} EXCLUDE_FROM_ALL ${Example})
  target_link_libraries(${ExampleName} ${PROJECT_NAME})

  add_dependencies(examples ${ExampleName})
endforeach ()






# set(LIBCURLPP_INCLUDE_DIR
#   ${IncludeFolder}
#   CACHE INTERNAL
#   "${PROJECT_NAME}: Libraries"
#   )

# set(LIBCURLPP_LIBS
#   ${PROJECT_NAME}
#   CACHE INTERNAL
#   "${PROJECT_NAME}: Libraries"
#   )

# list(APPEND LINK_DIRECTORIES "${CMAKE_CURRENT_BINARY_DIR}")
# set(LINK_DIRECTORIES "${LINK_DIRECTORIES}" PARENT_SCOPE)

# install(FILES  CreationStatic.hpp CreationStatic.inl  	CreationUsingNew.hpp CreationUsingNew.inl  	LifetimeDefault.hpp LifetimeDefault.inl  	LifetimeLibrary.hpp LifetimeLibrary.inl  	LifetimeWithLongevity.hpp LifetimeWithLongevity.inl  	PrivateMembers.hpp PrivateMembers.inl  	SingletonHolder.hpp SingletonHolder.inl  DESTINATION /include/utilspp/singleton)



# set(curlpp_LIB_SRCS ${sources})

# add_library(curlpp SHARED ${curlpp_LIB_SRCS})

# target_link_libraries(curlpp ${KDE4_KDECORE_LIBS} internal)

# set_target_properties(curlpp PROPERTIES VERSION 1.0.0 SOVERSION 1)
# install(TARGETS curlpp ${INSTALL_TARGETS_DEFAULT_ARGS})


# install(FILES  Binder.hpp   	FunctorHandler.hpp  	Functor.hpp  	FunctorImpl.hpp  	MemFunHandler.hpp DESTINATION /include/utilspp/functor)


# install(FILES  Easy.hpp Easy.inl  	Exception.hpp  	Form.hpp  	Info.hpp Info.inl  	Infos.hpp  	Multi.hpp  	Option.hpp Option.inl  	OptionBase.hpp  	Options.hpp  	Types.hpp  	cURLpp.hpp  	config.h.in  	config.h.sample  	config.win32.h  DESTINATION /include/curlpp)


# install(FILES  CurlHandle.hpp CurlHandle.inl  	OptionContainer.hpp OptionContainer.inl	OptionContainerType.hpp  	OptionList.hpp  	OptionSetter.hpp OptionSetter.inl  	SList.hpp  	buildconfig.h  	global.h DESTINATION /include/curlpp/internal)
