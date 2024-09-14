cmake_minimum_required( VERSION 3.13.0 )

target_sources( ${PROJECT_NAME}
    PRIVATE
	custom_pipe_serial.cpp
	custom_pipe_parallel.cpp
	payload.ixx
)