/**
 *
 * @copyright Copyright (c) 2021, Southwest Research Institute
 *
 * @par License
 * Software License Agreement (Apache License)
 * @par
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * @par
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <boost_plugin_loader/utils.h>
#include <boost_plugin_loader/plugin_loader.hpp>
#include "test_plugin_base.h"
#include "test_plugin_multiply.h"

TEST(BoostPluginLoaderUnit, LoadTestPluginUsingAnchor)  // NOLINT
{
  using boost_plugin_loader::addSymbolLibraryToSearchLibrariesEnv;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginBase;

  {
    PluginLoader plugin_loader;
    plugin_loader.search_libraries.clear();
    plugin_loader.search_paths.clear();
    plugin_loader.search_libraries_env = "PLUGIN_LOADER_UNIT_LIBRARY_NAMES_1_ENV";
    plugin_loader.search_paths_env = "PLUGIN_LOADER_UNIT_LIBRARY_PATHS_1_ENV";
    plugin_loader.search_system_folders = false;
    addSymbolLibraryToSearchLibrariesEnv(boost_plugin_loader::TestPluginMultiplyAnchor(),
                                         plugin_loader.search_libraries_env, plugin_loader.search_paths_env);
    EXPECT_EQ(plugin_loader.count(), 1);
    EXPECT_FALSE(plugin_loader.empty());
    EXPECT_TRUE(plugin_loader.isPluginAvailable("plugin"));
    auto plugin = plugin_loader.createInstance<TestPluginBase>("plugin");
    EXPECT_TRUE(plugin != nullptr);
    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_libraries.clear();
    plugin_loader.search_paths.clear();
    plugin_loader.search_libraries_env = "PLUGIN_LOADER_UNIT_LIBRARY_NAMES_2_ENV";
    plugin_loader.search_system_folders = false;
    addSymbolLibraryToSearchLibrariesEnv(boost_plugin_loader::TestPluginMultiplyAnchor(),
                                         plugin_loader.search_libraries_env);
    EXPECT_EQ(plugin_loader.count(), 1);
    EXPECT_FALSE(plugin_loader.empty());
    EXPECT_TRUE(plugin_loader.isPluginAvailable("plugin"));
    auto plugin = plugin_loader.createInstance<TestPluginBase>("plugin");
    EXPECT_TRUE(plugin != nullptr);
    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
