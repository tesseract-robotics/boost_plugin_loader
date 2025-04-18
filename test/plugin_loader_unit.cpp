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
#include "test_plugin_base.h"

TEST(BoostPluginLoaderUnit, Utils)  // NOLINT
{
  using namespace boost_plugin_loader;
  const std::string lib_name = std::string(PLUGINS);
  const std::string lib_dir = std::string(PLUGIN_DIR);
  const std::string symbol_name = "plugin";

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

  {
    const std::optional<boost::dll::shared_library> lib =
        loadLibrary(boost::filesystem::path("does_not_exist") / lib_name);
    EXPECT_FALSE(lib.has_value());
  }

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
  const std::optional<boost::dll::shared_library> lib = loadLibrary(boost::filesystem::path(lib_dir) / lib_name);
  EXPECT_TRUE(lib.has_value());

  {
    std::vector<std::string> sections = getAllAvailableSections(lib.value());  // NOLINT
    EXPECT_EQ(sections.size(), 1);
    EXPECT_EQ(sections.at(0), "TestBase");

    sections = getAllAvailableSections(lib.value(), true);  // NOLINT
    EXPECT_TRUE(sections.size() > 1);
  }

  {
    std::vector<std::string> symbols = getAllAvailableSymbols(lib.value(), "TestBase");  // NOLINT
    EXPECT_EQ(symbols.size(), 1);
    EXPECT_EQ(symbols.at(0), symbol_name);
  }

  {
    EXPECT_TRUE(lib.value().has(symbol_name));  // NOLINT
  }

// For some reason on Ubuntu 18.04 it does not search the current directory when only the library name is provided
#if BOOST_VERSION > 106800
  {
    EXPECT_TRUE(lib.value().has(symbol_name));  // NOLINT
  }
#endif

  {
    EXPECT_FALSE(lib.value().has("does_not_exist"));  // NOLINT
  }

  // Load the plugin
  EXPECT_NO_THROW(createSharedInstance<TestPluginBase>(symbol_name, lib.value()));        // NOLINT
  EXPECT_ANY_THROW(createSharedInstance<TestPluginBase>("does_not_exist", lib.value()));  // NOLINT
}

TEST(BoostPluginLoaderUnit, LoadTestPlugin)  // NOLINT
{
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginBase;

  {
    PluginLoader plugin_loader;
    plugin_loader.search_paths.insert(std::string(PLUGIN_DIR));
    plugin_loader.search_libraries.insert(std::string(PLUGINS));

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

// For some reason on Ubuntu 18.04 it does not search the current directory when only the library name is provided
#if BOOST_VERSION > 106800
  {
    PluginLoader plugin_loader;
    EXPECT_EQ(plugin_loader.count(), 0);
    EXPECT_TRUE(plugin_loader.empty());
    plugin_loader.search_libraries.insert(std::string(PLUGINS));
    EXPECT_EQ(plugin_loader.count(), 1);
    EXPECT_FALSE(plugin_loader.empty());

    EXPECT_TRUE(plugin_loader.isPluginAvailable("plugin"));
    auto plugin = plugin_loader.createInstance<TestPluginBase>("plugin");
    EXPECT_TRUE(plugin != nullptr);
    EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
  }
#endif

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_paths.insert("does_not_exist");
    plugin_loader.search_libraries.insert(std::string(PLUGINS));

    EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginBase>("plugin"));
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_libraries.insert(std::string(PLUGINS));

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable("does_not_exist"));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginBase>("does_not_exist"));
    }

    plugin_loader.search_system_folders = true;

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable("does_not_exist"));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginBase>("does_not_exist"));
    }
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_libraries.insert("does_not_exist");

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginBase>("plugin"));
    }

    plugin_loader.search_system_folders = true;

    {
      EXPECT_FALSE(plugin_loader.isPluginAvailable("plugin"));
      // Behavior change: used to return nullptr but now throws exception
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginBase>("plugin"));
    }
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_libraries.insert(std::string(PLUGINS));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    const std::vector<std::string> plugins = plugin_loader.getAvailablePlugins("TestBase");
    EXPECT_EQ(plugins.size(), 0);
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = true;
    plugin_loader.search_libraries.insert(std::string(PLUGINS));

    const std::vector<std::string> plugins = plugin_loader.getAvailablePlugins("TestBase");
    EXPECT_EQ(plugins.size(), 1);
  }

  {
    const PluginLoader plugin_loader;
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailablePlugins("TestBase"));
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailableSections("plugin"));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.isPluginAvailable("plugin"));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginBase>("plugin"));
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailablePlugins("TestBase"));
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailableSections("plugin"));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.isPluginAvailable("plugin"));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginBase>("plugin"));
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
