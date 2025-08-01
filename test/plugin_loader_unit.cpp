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
#include <set>
#include <vector>
#include <optional>
#include <cstdlib>  // NOLINT(misc-include-cleaner)

// Boost
#include <boost/version.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/path.hpp>

// Boost Plugin Loader
#include <boost_plugin_loader/utils.h>
#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/plugin_loader.hpp>
#include "test_plugin.h"

TEST(BoostPluginLoaderUnit, Utils)  // NOLINT
{
  using namespace boost_plugin_loader;
  const std::string lib_name = std::string(PLUGINS_MULTIPLY);
  const std::string lib_dir = std::string(PLUGIN_DIR);

  {
#ifndef _WIN32
    std::string env_var = "UNITTESTENV=a:b:c";
#else
    std::string env_var = "UNITTESTENV=a;b;c";
#endif
    putenv(env_var.data());  // NOLINT(misc-include-cleaner)
    std::set<std::string> s = parseEnvironmentVariableList("UNITTESTENV");
    std::vector<std::string> v(s.begin(), s.end());
    EXPECT_EQ(v[0], "a");
    EXPECT_EQ(v[1], "b");
    EXPECT_EQ(v[2], "c");

    s = parseEnvironmentVariableList("does_not_exist");
    EXPECT_TRUE(s.empty());
  }

  {  // Test getAllSearchPaths
#ifndef _WIN32
    std::string env_var = "UNITTESTENV=a:b:c";
#else
    std::string env_var = "UNITTESTENV=a;b;c";
#endif
    putenv(env_var.data());  // NOLINT(misc-include-cleaner)
    std::string search_paths_env = "UNITTESTENV";
    std::set<std::string> existing_search_paths;
    std::set<std::string> s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_EQ(s.size(), 3);
    search_paths_env.clear();
    s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_TRUE(s.empty());
    existing_search_paths = { "d", "e" };
    s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_EQ(s.size(), 2);
  }

  {  // Test getAllLibraryNames
#ifndef _WIN32
    std::string env_var = "UNITTESTENV=a:b:c";
#else
    std::string env_var = "UNITTESTENV=a;b;c";
#endif
    putenv(env_var.data());  // NOLINT(misc-include-cleaner)
    std::string search_paths_env = "UNITTESTENV";
    std::set<std::string> existing_search_paths;
    std::set<std::string> s = getAllLibraryNames(search_paths_env, existing_search_paths);
    EXPECT_EQ(s.size(), 3);
    search_paths_env.clear();
    s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_TRUE(s.empty());
    existing_search_paths = { "d", "e" };
    s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_EQ(s.size(), 2);
  }

#ifndef __APPLE__
  {
    const std::optional<boost::dll::shared_library> lib =
        loadLibrary(boost::filesystem::path("does_not_exist") / lib_name);
    EXPECT_FALSE(lib.has_value());
  }
#endif

  {
    const std::optional<boost::dll::shared_library> lib = loadLibrary(boost::filesystem::path(lib_dir) / "does_not_"
                                                                                                         "exist");
    EXPECT_FALSE(lib.has_value());
  }

  {
    const std::optional<boost::dll::shared_library> lib = loadLibrary(boost::filesystem::path("does_not_exist") / "does"
                                                                                                                  "_not"
                                                                                                                  "_"
                                                                                                                  "exis"
                                                                                                                  "t");
    EXPECT_FALSE(lib.has_value());
  }

  // Load the library
  const std::optional<boost::dll::shared_library> lib_opt = loadLibrary(boost::filesystem::path(lib_dir) / lib_name);
  EXPECT_TRUE(lib_opt.has_value());
  const boost::dll::shared_library& lib = lib_opt.value();  // NOLINT

  {
    std::vector<std::string> sections = getAllAvailableSections(lib);
    EXPECT_EQ(sections.size(), 1);
    EXPECT_EQ(sections.at(0), SECTION_MULTIPLY);

    sections = getAllAvailableSections(lib, true);
    EXPECT_TRUE(sections.size() > 1);
  }

  {
    std::vector<std::string> symbols = getAllAvailableSymbols(lib, SECTION_MULTIPLY);
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols.at(0), getSymbolName());
  }

  {
    EXPECT_TRUE(lib.has(getSymbolName()));
  }

// For some reason on Ubuntu 18.04 it does not search the current directory when only the library name is provided
#if BOOST_VERSION > 106800
  {
    EXPECT_TRUE(lib.has(getSymbolName()));
  }
#endif

  {
    EXPECT_FALSE(lib.has("does_not_exist"));
  }

  // Load the plugin
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  EXPECT_NO_THROW(createSharedInstance<TestPluginMultiply>(lib, getSymbolName()));
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  EXPECT_ANY_THROW(createSharedInstance<TestPluginMultiply>(lib, "does_not_exist"));
}

TEST(BoostPluginLoaderUnit, LoadTestPlugin)  // NOLINT
{
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  {
    PluginLoader plugin_loader;
    plugin_loader.search_paths.insert(std::string(PLUGIN_DIR));
    plugin_loader.search_libraries.insert(std::string(PLUGINS_MULTIPLY));

    EXPECT_TRUE(plugin_loader.isPluginAvailable(getSymbolName()));
    auto plugin = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
    EXPECT_TRUE(plugin != nullptr);
    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);

    std::vector<std::string> sections = plugin_loader.getAvailableSections();
    EXPECT_EQ(sections.size(), 1);
    EXPECT_EQ(sections.at(0), SECTION_MULTIPLY);

    sections = plugin_loader.getAvailableSections(true);
    EXPECT_TRUE(sections.size() > 1);

    std::vector<std::string> symbols = plugin_loader.getAvailablePlugins<TestPluginMultiply>();
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols.at(0), getSymbolName());

    symbols = plugin_loader.getAvailablePlugins(SECTION_MULTIPLY);
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols.at(0), getSymbolName());
  }

  {  // Use full path
    PluginLoader plugin_loader;
    const std::string full_path = boost_plugin_loader::decorate(std::string(PLUGINS_MULTIPLY), std::string(PLUGIN_DIR));
    plugin_loader.search_libraries.insert(full_path);

    EXPECT_TRUE(plugin_loader.isPluginAvailable(getSymbolName()));
    auto plugin = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
    EXPECT_TRUE(plugin != nullptr);
    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);

    std::vector<std::string> sections = plugin_loader.getAvailableSections();
    EXPECT_EQ(sections.size(), 1);
    EXPECT_EQ(sections.at(0), SECTION_MULTIPLY);

    sections = plugin_loader.getAvailableSections(true);
    EXPECT_TRUE(sections.size() > 1);

    std::vector<std::string> symbols = plugin_loader.getAvailablePlugins<TestPluginMultiply>();
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols.at(0), getSymbolName());

    symbols = plugin_loader.getAvailablePlugins(SECTION_MULTIPLY);
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols.at(0), getSymbolName());
  }

// For some reason on Ubuntu 18.04 it does not search the current directory when only the library name is provided
#if BOOST_VERSION > 106800
  {
    PluginLoader plugin_loader;
    EXPECT_EQ(plugin_loader.count(), 0);
    EXPECT_TRUE(plugin_loader.empty());
    plugin_loader.search_libraries.insert(std::string(PLUGINS_MULTIPLY));
    EXPECT_EQ(plugin_loader.count(), 1);
    EXPECT_FALSE(plugin_loader.empty());

    EXPECT_TRUE(plugin_loader.isPluginAvailable(getSymbolName()));
    auto plugin = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
    EXPECT_TRUE(plugin != nullptr);
    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
  }
#endif

#ifndef __APPLE__
  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_paths.insert("does_not_exist");
    plugin_loader.search_libraries.insert(std::string(PLUGINS_MULTIPLY));

    EXPECT_FALSE(plugin_loader.isPluginAvailable(getSymbolName()));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
  }
#endif

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_libraries.insert(std::string(PLUGINS_MULTIPLY));

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable("does_not_exist"));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>("does_not_exist"));
    }

    plugin_loader.search_system_folders = true;

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable("does_not_exist"));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>("does_not_exist"));
    }
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_libraries.insert("does_not_exist");

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable(getSymbolName()));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
    }

    plugin_loader.search_system_folders = true;

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable(getSymbolName()));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
    }
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_libraries.insert(std::string(PLUGINS_MULTIPLY));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    const std::vector<std::string> plugins = plugin_loader.getAvailablePlugins(SECTION_MULTIPLY);
    EXPECT_EQ(plugins.size(), 0);
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = true;
    plugin_loader.search_libraries.insert(std::string(PLUGINS_MULTIPLY));

    const std::vector<std::string> plugins = plugin_loader.getAvailablePlugins(SECTION_MULTIPLY);
    EXPECT_EQ(plugins.size(), 1);
  }

  {
    const PluginLoader plugin_loader;
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailablePlugins(SECTION_MULTIPLY));
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailableSections());
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.isPluginAvailable(getSymbolName()));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailablePlugins(SECTION_MULTIPLY));
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailableSections());
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.isPluginAvailable(getSymbolName()));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
