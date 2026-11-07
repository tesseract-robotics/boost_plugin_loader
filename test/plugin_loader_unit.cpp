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
#include <future>
#include <cstdlib>  // NOLINT(misc-include-cleaner)
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

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
    std::string env_var = "UNITTESTENV=c:b:a";
#else
    std::string env_var = "UNITTESTENV=c;b;a";
#endif
    putenv(env_var.data());  // NOLINT(misc-include-cleaner)
    std::vector<std::string> v = parseEnvironmentVariableList("UNITTESTENV");
    EXPECT_EQ(v[0], "c");
    EXPECT_EQ(v[1], "b");
    EXPECT_EQ(v[2], "a");

    v = parseEnvironmentVariableList("does_not_exist");
    EXPECT_TRUE(v.empty());
  }

  {  // Test getAllSearchPaths
#ifndef _WIN32
    std::string env_var = "UNITTESTENV=c:b:a";
#else
    std::string env_var = "UNITTESTENV=c;b;a";
#endif
    putenv(env_var.data());  // NOLINT(misc-include-cleaner)
    std::string search_paths_env = "UNITTESTENV";
    std::vector<std::string> existing_search_paths;
    std::vector<std::string> s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_EQ(s.size(), 3);
    search_paths_env.clear();
    s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_TRUE(s.empty());
    existing_search_paths.insert(existing_search_paths.end(), { "d", "e" });
    s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_EQ(s.size(), 2);
  }

  {  // Test getAllLibraryNames
#ifndef _WIN32
    std::string env_var = "UNITTESTENV=c:b:a";
#else
    std::string env_var = "UNITTESTENV=c;b;a";
#endif
    putenv(env_var.data());  // NOLINT(misc-include-cleaner)
    std::string search_paths_env = "UNITTESTENV";
    std::vector<std::string> existing_search_paths;
    std::vector<std::string> s = getAllLibraryNames(search_paths_env, existing_search_paths);
    EXPECT_EQ(s.size(), 3);
    search_paths_env.clear();
    s = getAllSearchPaths(search_paths_env, existing_search_paths);
    EXPECT_TRUE(s.empty());
    existing_search_paths.insert(existing_search_paths.end(), { "d", "e" });
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
    EXPECT_EQ(sections.at(0), TestPluginMultiply::getSection());

    sections = getAllAvailableSections(lib, true);
    EXPECT_TRUE(sections.size() > 1);
  }

  {
    std::vector<std::string> symbols = getAllAvailableSymbols(lib, TestPluginMultiply::getSection());
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
    plugin_loader.search_paths.emplace_back(PLUGIN_DIR);
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);

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

  {  // Use full path
    PluginLoader plugin_loader;
    const std::string full_path = boost_plugin_loader::decorate(std::string(PLUGINS_MULTIPLY), std::string(PLUGIN_DIR));
    plugin_loader.search_libraries.push_back(full_path);

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

// For some reason on Ubuntu 18.04 it does not search the current directory when only the library name is provided
#if BOOST_VERSION > 106800
  {
    PluginLoader plugin_loader;
    EXPECT_EQ(plugin_loader.count(), 0);
    EXPECT_TRUE(plugin_loader.empty());
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);
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
    plugin_loader.search_paths.emplace_back("does_not_exist");
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);

    EXPECT_FALSE(plugin_loader.isPluginAvailable(getSymbolName()));
    // Behavior change: used to return nullptr but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
  }
#endif

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = false;
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);

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
    plugin_loader.search_libraries.emplace_back("does_not_exist");

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
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    const std::vector<std::string> plugins = plugin_loader.getAvailablePlugins(TestPluginMultiply::getSection());
    EXPECT_EQ(plugins.size(), 0);
  }

  {
    PluginLoader plugin_loader;
    plugin_loader.search_system_folders = true;
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);

    const std::vector<std::string> plugins = plugin_loader.getAvailablePlugins(TestPluginMultiply::getSection());
    EXPECT_EQ(plugins.size(), 1);
  }

  {
    const PluginLoader plugin_loader;
    // Behavior change: used to return empty vector but now throws exception
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.getAvailablePlugins(TestPluginMultiply::getSection()));
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
    EXPECT_ANY_THROW(plugin_loader.getAvailablePlugins(TestPluginMultiply::getSection()));
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

TEST(BoostPluginLoaderUnit, AcquireLibraryLifetimeTokens)  // NOLINT
{
  std::vector<boost_plugin_loader::LibraryLifetimeToken> tokens;
  {
    boost_plugin_loader::PluginLoader plugin_loader;
    plugin_loader.search_paths.emplace_back(PLUGIN_DIR);
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);

    tokens = plugin_loader.acquireLibraryLifetimeTokens();
    ASSERT_EQ(tokens.size(), 1);
    EXPECT_FALSE(tokens.front().library_path.empty());
    EXPECT_NE(tokens.front().lifetime, nullptr);

    plugin_loader.clear();
    EXPECT_NE(tokens.front().lifetime, nullptr);
  }

  // The token retains its own library ownership after the loader is destroyed.
  EXPECT_NE(tokens.front().lifetime, nullptr);
}

TEST(BoostPluginLoaderUnit, LoadTestPluginsSameSymbolDifferentSections)  // NOLINT
{
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginAdd;
  using boost_plugin_loader::TestPluginMultiply;

  // Both the plugin `TestPluginAddImpl` and `TestPluginMultiplyImpl` were exported with the same symbol name (defined
  // by the target compile defintion __SYMBOL_NAME__) but in different sections and in different libraries Test to make
  // sure the symbol from the wrong library does not get loaded

  // Try to load an instance of `TestPluginAddImpl` from the library in which `TestPluginMultiplyImpl` was defined
  {
    PluginLoader plugin_loader;
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);
    plugin_loader.search_paths.emplace_back(PLUGIN_DIR);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginAdd>(getSymbolName()));
  }

  // Try to load an instance of `TestPluginMultiplyImpl` from the library in which `TestPluginAddImpl` was defined
  {
    PluginLoader plugin_loader;
    plugin_loader.search_libraries.emplace_back(PLUGINS_ADD);
    plugin_loader.search_paths.emplace_back(PLUGIN_DIR);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_ANY_THROW(plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
  }

  // Given both libraries, correctly load and use each plugin, even though they share the same symbol name
  {
    PluginLoader plugin_loader;
    plugin_loader.search_libraries.emplace_back(PLUGINS_ADD);
    plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);
    plugin_loader.search_paths.emplace_back(PLUGIN_DIR);

    // Load and use a multiply plugin
    {
      std::shared_ptr<TestPluginMultiply> plugin;
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_NO_THROW(plugin = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName()));
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_NEAR(plugin->multiply(3.0, 3.0), 9.0, 1.0e-6);
    }

    // Load and use an add plugin
    {
      std::shared_ptr<TestPluginAdd> plugin;
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_NO_THROW(plugin = plugin_loader.createInstance<TestPluginAdd>(getSymbolName()));
      // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
      EXPECT_NEAR(plugin->add(3.0, 3.0), 6.0, 1.0e-6);
    }
  }
}

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

TEST(BoostPluginLoaderUnit, InstancesShareTheCachedLibrary)  // NOLINT
{
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  PluginLoader plugin_loader;
  plugin_loader.search_paths.emplace_back(PLUGIN_DIR);
  plugin_loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);

  auto first = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
  auto second = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
  std::vector<boost_plugin_loader::LibraryLifetimeToken> tokens = plugin_loader.acquireLibraryLifetimeTokens();
  ASSERT_EQ(tokens.size(), 1);

  // One owner for all three: each aliases the cached library handle. Copying boost::dll::shared_library instead
  // re-resolves the handle's path, which on macOS runs dlopen/dlclose under dyld's process-wide lock (#45).
  EXPECT_FALSE(first.owner_before(second) || second.owner_before(first));
  EXPECT_FALSE(first.owner_before(tokens.front().lifetime) || tokens.front().lifetime.owner_before(first));

  // The shared owner still keeps the library loaded once the loader lets go of it (#13).
  plugin_loader.clear();
  EXPECT_NEAR(first->multiply(5, 5), 25, 1e-8);
}

TEST(BoostPluginLoaderUnit, CreateSharedInstanceFromLoadedLibrary)  // NOLINT
{
  using boost_plugin_loader::LoadedLibrary;
  using boost_plugin_loader::TestPluginMultiply;

  const boost::filesystem::path path =
      boost_plugin_loader::decorate(std::string(PLUGINS_MULTIPLY), std::string(PLUGIN_DIR));
  std::optional<boost::dll::shared_library> lib = boost_plugin_loader::loadLibrary(path);
  ASSERT_TRUE(lib.has_value());
  const LoadedLibrary loaded{ std::make_shared<boost::dll::shared_library>(std::move(lib.value())), path,
                              std::make_shared<boost_plugin_loader::SymbolIndex>() };

  auto plugin = boost_plugin_loader::createSharedInstance<TestPluginMultiply>(loaded, getSymbolName());
  EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
  EXPECT_FALSE(plugin.owner_before(loaded.library) || loaded.library.owner_before(plugin));
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  EXPECT_ANY_THROW(boost_plugin_loader::createSharedInstance<TestPluginMultiply>(loaded, "does_not_exist"));

  // A second lookup of each name is answered from the index: the same symbol, and the miss still throws.
  auto again = boost_plugin_loader::createSharedInstance<TestPluginMultiply>(loaded, getSymbolName());
  EXPECT_EQ(again.get(), plugin.get());
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  EXPECT_ANY_THROW(boost_plugin_loader::createSharedInstance<TestPluginMultiply>(loaded, "does_not_exist"));
  EXPECT_EQ(loaded.symbols->addressResolutions(), 2);
}

namespace
{
/** @brief The registry key of the multiply test library, found through PLUGIN_DIR: the path it is loaded from */
std::string multiplyLibraryKey()
{
  return (boost::filesystem::path(PLUGIN_DIR) / PLUGINS_MULTIPLY).string();
}

/** @brief A loader for the multiply test library through PLUGIN_DIR, using @p registry */
boost_plugin_loader::PluginLoader multiplyLoader(
    std::shared_ptr<boost_plugin_loader::LibraryRegistry> registry = boost_plugin_loader::LibraryRegistry::global())
{
  boost_plugin_loader::PluginLoader loader(std::move(registry));
  loader.search_paths.emplace_back(PLUGIN_DIR);
  loader.search_libraries.emplace_back(PLUGINS_MULTIPLY);
  return loader;
}

/** @brief The boost::dll::shared_library object @p loader uses for the multiply test library */
const void* libraryObject(const boost_plugin_loader::PluginLoader& loader)
{
  const std::vector<boost_plugin_loader::LibraryLifetimeToken> tokens = loader.acquireLibraryLifetimeTokens();
  EXPECT_EQ(tokens.size(), 1);
  return tokens.front().lifetime.get();
}
}  // namespace

TEST(BoostPluginLoaderUnit, LoadersShareTheProcessRegistry)  // NOLINT
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::LoadedLibrary;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  const PluginLoader first_loader = multiplyLoader();
  EXPECT_EQ(PluginLoader().registry(), LibraryRegistry::global());
  auto first = first_loader.createInstance<TestPluginMultiply>(getSymbolName());

  const std::optional<LoadedLibrary> entry = LibraryRegistry::global()->find(multiplyLibraryKey());
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(libraryObject(first_loader), entry->library.get());

  // A second loader resolves nothing: it uses the entry the first one loaded, with the same library object and the
  // same symbol index (a new load would make new ones).
  const PluginLoader second_loader = multiplyLoader();
  auto second = second_loader.createInstance<TestPluginMultiply>(getSymbolName());
  EXPECT_EQ(libraryObject(second_loader), entry->library.get());

  const std::optional<LoadedLibrary> again = LibraryRegistry::global()->find(multiplyLibraryKey());
  ASSERT_TRUE(again.has_value());
  EXPECT_EQ(again->library, entry->library);
  EXPECT_EQ(again->symbols, entry->symbols);
  EXPECT_EQ(first.get(), second.get());
}

TEST(BoostPluginLoaderUnit, InjectedRegistryIsIsolated)  // NOLINT
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::LoadedLibrary;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  const PluginLoader global_loader = multiplyLoader();
  global_loader.createInstance<TestPluginMultiply>(getSymbolName());
  const std::optional<LoadedLibrary> global_entry = LibraryRegistry::global()->find(multiplyLibraryKey());
  ASSERT_TRUE(global_entry.has_value());

  // An injected registry starts empty: it does not see the global registry's entry...
  auto own = std::make_shared<LibraryRegistry>();
  EXPECT_FALSE(own->find(multiplyLibraryKey()).has_value());

  const PluginLoader own_loader = multiplyLoader(own);
  EXPECT_EQ(own_loader.registry(), own);
  auto from_own = own_loader.createInstance<TestPluginMultiply>(getSymbolName());
  EXPECT_NEAR(from_own->multiply(5, 5), 25, 1e-8);

  // ...it loads its own entry...
  const std::optional<LoadedLibrary> own_entry = own->find(multiplyLibraryKey());
  ASSERT_TRUE(own_entry.has_value());
  EXPECT_NE(own_entry->library, global_entry->library);
  EXPECT_NE(own_entry->symbols, global_entry->symbols);
  EXPECT_EQ(libraryObject(own_loader), own_entry->library.get());

  // ...and the global registry does not see it.
  const std::optional<LoadedLibrary> global_after = LibraryRegistry::global()->find(multiplyLibraryKey());
  ASSERT_TRUE(global_after.has_value());
  EXPECT_EQ(global_after->library, global_entry->library);
  EXPECT_EQ(libraryObject(global_loader), global_entry->library.get());

  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  EXPECT_THROW(PluginLoader{ nullptr }, boost_plugin_loader::PluginLoaderException);
}

TEST(BoostPluginLoaderUnit, ClearKeepsRegistryEntries)  // NOLINT
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::LoadedLibrary;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  auto registry = std::make_shared<LibraryRegistry>();
  PluginLoader plugin_loader = multiplyLoader(registry);
  auto before = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
  const std::optional<LoadedLibrary> entry = registry->find(multiplyLibraryKey());
  ASSERT_TRUE(entry.has_value());

  // The registry holds strong references: clear() empties only the loader's view, and the next call reuses the entry.
  plugin_loader.clear();
  const std::optional<LoadedLibrary> after_clear = registry->find(multiplyLibraryKey());
  ASSERT_TRUE(after_clear.has_value());
  EXPECT_EQ(after_clear->library, entry->library);
  EXPECT_EQ(after_clear->symbols, entry->symbols);

  auto after = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
  EXPECT_EQ(libraryObject(plugin_loader), entry->library.get());
  EXPECT_EQ(before.get(), after.get());
  EXPECT_NEAR(before->multiply(5, 5), 25, 1e-8);
}

TEST(BoostPluginLoaderUnit, LoaderSelfAssignmentKeepsState)  // NOLINT
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  auto registry = std::make_shared<LibraryRegistry>();
  PluginLoader plugin_loader = multiplyLoader(registry);
  plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
  const std::vector<std::string> search_paths = plugin_loader.search_paths;
  const std::vector<std::string> search_libraries = plugin_loader.search_libraries;
  const void* library = libraryObject(plugin_loader);

  // Assigning a loader to itself must leave it unchanged. Both operators lock this and other's mutex together, so
  // without a self-assignment check they would lock one mutex twice.
  PluginLoader& alias = plugin_loader;
  plugin_loader = alias;
  EXPECT_EQ(plugin_loader.registry(), registry);
  EXPECT_EQ(plugin_loader.search_paths, search_paths);
  EXPECT_EQ(plugin_loader.search_libraries, search_libraries);
  EXPECT_EQ(libraryObject(plugin_loader), library);

  plugin_loader = std::move(alias);  // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
  EXPECT_EQ(plugin_loader.registry(), registry);
  EXPECT_EQ(plugin_loader.search_paths, search_paths);
  EXPECT_EQ(plugin_loader.search_libraries, search_libraries);
  EXPECT_EQ(libraryObject(plugin_loader), library);
  auto plugin = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
  EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
}

TEST(BoostPluginLoaderUnit, LoaderViewKeepsRegistryLibraryAlive)  // NOLINT
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  // A loader's view holds its own reference counts, each holding the registry's: an instance keeps its library loaded
  // after both the loader and an injected registry are gone (#13).
  std::shared_ptr<TestPluginMultiply> plugin;
  std::weak_ptr<boost::dll::shared_library> registered;
  {
    auto registry = std::make_shared<LibraryRegistry>();
    const PluginLoader plugin_loader = multiplyLoader(registry);
    plugin = plugin_loader.createInstance<TestPluginMultiply>(getSymbolName());
    registered = registry->find(multiplyLibraryKey())->library;
  }
  EXPECT_FALSE(registered.expired());
  EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
  plugin.reset();
  EXPECT_TRUE(registered.expired());
}

TEST(BoostPluginLoaderUnit, SymbolAddressesResolvedOncePerLibrary)  // NOLINT
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::LoadedLibrary;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  // A registry of its own, so the count starts at this test.
  auto registry = std::make_shared<LibraryRegistry>();
  const PluginLoader first_loader = multiplyLoader(registry);
  auto first = first_loader.createInstance<TestPluginMultiply>(getSymbolName());
  const std::optional<LoadedLibrary> entry = registry->find(multiplyLibraryKey());
  ASSERT_TRUE(entry.has_value());
  EXPECT_EQ(entry->symbols->addressResolutions(), 1);

  // The same symbol again, through the same loader and through another one: no further dlsym.
  auto again = first_loader.createInstance<TestPluginMultiply>(getSymbolName());
  const PluginLoader second_loader = multiplyLoader(registry);
  auto second = second_loader.createInstance<TestPluginMultiply>(getSymbolName());
  EXPECT_TRUE(second_loader.isPluginAvailable(getSymbolName()));
  EXPECT_EQ(entry->symbols->addressResolutions(), 1);
  EXPECT_EQ(again.get(), first.get());
  EXPECT_EQ(second.get(), first.get());

  // A missing name is resolved once too, and every lookup of it still fails.
  EXPECT_FALSE(first_loader.isPluginAvailable("does_not_exist"));
  EXPECT_FALSE(second_loader.isPluginAvailable("does_not_exist"));
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
  EXPECT_ANY_THROW(boost_plugin_loader::createSharedInstance<TestPluginMultiply>(entry.value(), "does_not_exist"));
  EXPECT_EQ(entry->symbols->addressResolutions(), 2);
}

TEST(BoostPluginLoaderUnit, SymbolAddressesConcurrentFirstLookup)  // NOLINT
{
  using boost_plugin_loader::LibraryRegistry;
  using boost_plugin_loader::PluginLoader;
  using boost_plugin_loader::TestPluginMultiply;

  // Loaders on several threads make the first lookups together: every instance is the same object, and the index
  // settles on one address however many threads resolved it before the first was published.
  auto registry = std::make_shared<LibraryRegistry>();
  multiplyLoader(registry).acquireLibraryLifetimeTokens();  // load once, so the race is on the lookups only
  constexpr int THREADS = 8;
  std::vector<std::future<const void*>> results;
  results.reserve(THREADS);
  for (int i = 0; i < THREADS; ++i)
    results.push_back(std::async(std::launch::async, [registry] {
      const PluginLoader loader = multiplyLoader(registry);
      auto plugin = loader.createInstance<TestPluginMultiply>(getSymbolName());
      EXPECT_NEAR(plugin->multiply(5, 5), 25, 1e-8);
      return static_cast<const void*>(plugin.get());
    }));
  std::set<const void*> addresses;
  for (auto& result : results)
    addresses.insert(result.get());
  EXPECT_EQ(addresses.size(), 1);
  const std::size_t resolutions = registry->find(multiplyLibraryKey())->symbols->addressResolutions();
  EXPECT_GE(resolutions, 1);
  EXPECT_LE(resolutions, static_cast<std::size_t>(THREADS));
}
