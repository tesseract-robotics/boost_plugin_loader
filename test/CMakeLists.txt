find_gtest()

add_library(${PROJECT_NAME}_test_plugin INTERFACE test_plugin.h)
target_compile_definitions(${PROJECT_NAME}_test_plugin INTERFACE
  SECTION_MULTIPLY="mult"
  SECTION_ADD="add"
  __SYMBOL_NAME__=plugin)

add_library(${PROJECT_NAME}_test_plugin_multiply test_plugin_multiply.cpp)
target_link_libraries(${PROJECT_NAME}_test_plugin_multiply PUBLIC ${PROJECT_NAME} ${PROJECT_NAME}_test_plugin Boost::boost)
target_compile_definitions(${PROJECT_NAME}_test_plugin_multiply PUBLIC ${COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_test_plugin_multiply ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_test_plugin_multiply PUBLIC VERSION 17)

add_library(${PROJECT_NAME}_test_plugin_add test_plugin_add.cpp)
target_link_libraries(${PROJECT_NAME}_test_plugin_add PUBLIC ${PROJECT_NAME} ${PROJECT_NAME}_test_plugin Boost::boost)
target_compile_definitions(${PROJECT_NAME}_test_plugin_add PUBLIC ${COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_test_plugin_add ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_test_plugin_add PUBLIC VERSION 17)

add_executable(${PROJECT_NAME}_plugin_loader_unit plugin_loader_unit.cpp)
target_link_libraries(${PROJECT_NAME}_plugin_loader_unit PRIVATE GTest::GTest GTest::Main ${PROJECT_NAME} ${PROJECT_NAME}_test_plugin)
target_compile_definitions(${PROJECT_NAME}_plugin_loader_unit PRIVATE ${COMPILE_DEFINITIONS})
target_compile_definitions(${PROJECT_NAME}_plugin_loader_unit PRIVATE PLUGIN_DIR="${CMAKE_CURRENT_BINARY_DIR}"
                                                                      PLUGINS_MULTIPLY="${PROJECT_NAME}_test_plugin_multiply"
                                                                      PLUGINS_ADD="${PROJECT_NAME}_test_plugin_add")
target_clang_tidy(${PROJECT_NAME}_plugin_loader_unit ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_plugin_loader_unit PUBLIC VERSION 17)
target_code_coverage(
  ${PROJECT_NAME}_plugin_loader_unit
  PRIVATE
  ALL
  ENABLE ${ENABLE_CODE_COVERAGE})
add_gtest_discover_tests(${PROJECT_NAME}_plugin_loader_unit)
add_dependencies(${PROJECT_NAME}_plugin_loader_unit ${PROJECT_NAME})
add_dependencies(run_tests ${PROJECT_NAME}_plugin_loader_unit)

add_executable(${PROJECT_NAME}_plugin_loader_anchor_unit plugin_loader_anchor_unit.cpp)
target_link_libraries(
  ${PROJECT_NAME}_plugin_loader_anchor_unit
  PRIVATE GTest::GTest
          GTest::Main
          ${PROJECT_NAME}
          ${PROJECT_NAME}_test_plugin
          ${PROJECT_NAME}_test_plugin_multiply)
target_compile_definitions(${PROJECT_NAME}_plugin_loader_anchor_unit PRIVATE ${COMPILE_DEFINITIONS})
target_clang_tidy(${PROJECT_NAME}_plugin_loader_anchor_unit ENABLE ${ENABLE_CLANG_TIDY})
target_cxx_version(${PROJECT_NAME}_plugin_loader_anchor_unit PUBLIC VERSION 17)
target_code_coverage(
  ${PROJECT_NAME}_plugin_loader_anchor_unit
  PRIVATE
  ALL
  ENABLE ${ENABLE_CODE_COVERAGE})
add_gtest_discover_tests(${PROJECT_NAME}_plugin_loader_anchor_unit)
add_dependencies(${PROJECT_NAME}_plugin_loader_anchor_unit ${PROJECT_NAME})
add_dependencies(run_tests ${PROJECT_NAME}_plugin_loader_anchor_unit)

install(
  TARGETS ${PROJECT_NAME}_test_plugin_multiply
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
