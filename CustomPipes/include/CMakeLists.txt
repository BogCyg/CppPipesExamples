cmake_minimum_required( VERSION 3.13.0 )

target_sources( ${PROJECT_NAME}
    PUBLIC
        helpers.h
	range.h
)

target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR} )






