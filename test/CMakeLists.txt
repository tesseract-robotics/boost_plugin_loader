find_gtest()

add_library(${PROJECT_NAME}_test_plugin_multiply test_plugin_multiply.cpp)
target_link_libraries(${PROJECT_NAME}_test_plugin_multiply PUBLIC ${PROJECT_NAME} Boost::boost)
target_compile_definitions(${PROJECT_NAME}_test_plugin_multiply PUBLIC ${COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_test_plugin_multiply ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_test_plugin_multiply PUBLIC VERSION 17)

add_executable(${PROJECT_NAME}_plugin_loader_unit plugin_loader_unit.cpp)
target_link_libraries(${PROJECT_NAME}_plugin_loader_unit PRIVATE GTest::GTest GTest::Main ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME}_test_plugin_multiply PRIVATE ${COMPILE_DEFINITIONS})
target_compile_definitions(${PROJECT_NAME}_plugin_loader_unit PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                                                                      PLUGINS="${PROJECT_NAME}_test_plugin_multiply")
target_clang_tidy(${PROJECT_NAME}_plugin_loader_unit ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_plugin_loader_unit PUBLIC VERSION 17)
target_code_coverage(${PROJECT_NAME}_plugin_loader_unit ALL ENABLE ${ENABLE_CODE_COVERAGE})
add_gtest_discover_tests(${PROJECT_NAME}_plugin_loader_unit)
add_dependencies(${PROJECT_NAME}_plugin_loader_unit ${PROJECT_NAME})
add_dependencies(run_tests ${PROJECT_NAME}_plugin_loader_unit)

install(
  TARGETS ${PROJECT_NAME}_test_plugin_multiply
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
