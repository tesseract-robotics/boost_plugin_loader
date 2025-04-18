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

// GTest
#include <gtest/gtest.h>

// STD
#include <string>
#include <vector>
#include <cstdlib>  // NOLINT(misc-include-cleaner)

// Boost Plugin Loader
#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/plugin_loader.hpp>  // NOLINT(misc-include-cleaner)
#include "test_plugin_base.h"
#include "test_plugin_multiply.h"

TEST(BoostPluginLoaderAnchorUnit, LoadTestPlugin)  // NOLINT
{
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginBase;

  PluginLoader::addSymbolLibraryToSearchLibrariesEnv(boost_plugin_loader::TestPluginMultiplyAnchor(), "UNITTESTENV");
  PluginLoader plugin_loader;
  plugin_loader.search_libraries_env = "UNITTESTENV";

  EXPECT_TRUE(plugin_loader.isPluginAvailable("plugin"));
  auto plugin = plugin_loader.createInstance<TestPluginBase>("plugin");
  EXPECT_TRUE(plugin != nullptr);
  EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);

  std::vector<std::string> sections = plugin_loader.getAvailableSections();
  EXPECT_EQ(sections.size(), 1);
  EXPECT_EQ(sections.at(0), "TestBase");

  sections = plugin_loader.getAvailableSections(true);
  EXPECT_TRUE(sections.size() > 1);

  std::vector<std::string> symbols = plugin_loader.getAvailablePlugins<TestPluginBase>();
  EXPECT_EQ(symbols.size(), 1);
  EXPECT_EQ(symbols.at(0), "plugin");

  symbols = plugin_loader.getAvailablePlugins("TestBase");
  EXPECT_EQ(symbols.size(), 1);
  EXPECT_EQ(symbols.at(0), "plugin");
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
