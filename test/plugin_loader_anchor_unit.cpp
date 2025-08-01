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
#include <boost_plugin_loader/utils.h>
#include "test_plugin.h"
#include "test_plugin_multiply.h"

TEST(BoostPluginLoaderAnchorUnit, LoadTestPlugin)  // NOLINT
{
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  boost_plugin_loader::addSymbolLibraryToSearchLibrariesEnv(boost_plugin_loader::TestPluginMultiplyImplAnchor(), "UNITT"
                                                                                                                 "ESTEN"
                                                                                                                 "V");
  PluginLoader plugin_loader;
  plugin_loader.search_libraries_env = "UNITTESTENV";

  EXPECT_TRUE(plugin_loader.isPluginAvailable(getSymbolName()));
  auto plugin = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
  EXPECT_TRUE(plugin != nullptr);
  EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);

  std::vector<std::string> sections = plugin_loader.getAvailableSections();
  EXPECT_EQ(sections.size(), 1);
  EXPECT_EQ(sections.at(0), TestPluginMultiply::getSection());

  sections = plugin_loader.getAvailableSections(true);
  EXPECT_TRUE(sections.size() > 1);

  std::vector<std::string> symbols = plugin_loader.getAvailablePlugins<TestPluginMultiply>();
  EXPECT_EQ(symbols.size(), 1);
  EXPECT_EQ(symbols.at(0), getSymbolName());

  symbols = plugin_loader.getAvailablePlugins(TestPluginMultiply::getSection());
  EXPECT_EQ(symbols.size(), 1);
  EXPECT_EQ(symbols.at(0), getSymbolName());
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
