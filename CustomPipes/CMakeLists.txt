cmake_minimum_required( VERSION 3.13.0 )

# For a new project it is sufficient to change only its name in the following line
set( PROJECT_NAME CustomPipes )

project( ${PROJECT_NAME} )

set( CMAKE_BUILD_TYPE Debug )
#set( CMAKE_BUILD_TYPE Release )



if( WIN32 )
	set( CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /GR /EHsc /std:c++latest /D_UNICODE /DUNICODE" )
	set( CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1 /std:c++latest /D_UNICODE /DUNICODE" )
	message( "Win settings chosen..." )
elseif( ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin" )
	set( CMAKE_CXX_FLAGS "-std=c++latest -Wall" )
	set( CMAKE_CXX_FLAGS_DEBUG "-g -std=c++latest -Wall" )
	message( "Mac settings chosen..." )
elseif( UNIX )
	set( CMAKE_CXX_FLAGS "-std=c++latest -Wall" )
	set( CMAKE_CXX_FLAGS_DEBUG "-g -std=c++latest -Wall" )
	message( "Linux settings chosen..." )
endif()




add_executable( ${PROJECT_NAME} "" )



add_subdirectory( src )
add_subdirectory( include )


# This works in VS only when at this level of CMakeLists.txt
set_property( GLOBAL PROPERTY USE_FOLDERS ON )
source_group( "Source Files/Pipes" src/pipes )



# Set the default project 
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME} )


message( "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}" )
