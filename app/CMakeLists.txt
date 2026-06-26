cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(specialized_test)

target_include_directories(app PRIVATE include)

target_sources(app PRIVATE
	src/main.c
	src/app_config_model.c
	src/zephyr_app_config.c
	src/app_logic.c
	src/can_formatter.c
	src/runtime.c
)
