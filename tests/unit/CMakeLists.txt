cmake_minimum_required(VERSION 3.20.0)

if(BOARD STREQUAL "unit_testing" OR BOARD STREQUAL "unit_testing/unit_testing")
  find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})
  project(specialized_test_unit)

  target_include_directories(testbinary PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../app/include)

  target_sources(testbinary PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/src/app_config_model.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/src/app_logic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/src/can_formatter.c
    src/test_app_config_model.c
    src/test_app_logic.c
    src/test_can_formatter.c
  )
else()
  message(FATAL_ERROR "tests/unit only supports the unit_testing platform")
endif()
