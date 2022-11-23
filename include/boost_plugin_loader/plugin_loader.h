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
#ifndef BOOST_PLUGIN_LOADER_PLUGIN_LOADER_H
#define BOOST_PLUGIN_LOADER_PLUGIN_LOADER_H

#include <set>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

/** @brief Macro for explicitly template instantiating a plugin loader for a given base class */
#define INSTANTIATE_PLUGIN_LOADER(PluginBase)                                                                          \
  template std::vector<std::string> boost_plugin_loader::PluginLoader::getAvailablePlugins<PluginBase>() const;        \
  template std::shared_ptr<PluginBase> boost_plugin_loader::PluginLoader::createInstance(const std::string&) const;

namespace boost_plugin_loader
{
/** @brief Used to test for getSection method for getAvailablePlugins */
template <typename T>
struct has_getSection
{
  template <typename U>
  static constexpr decltype(std::declval<U>().getSection(), bool()) test_getSection(int)
  {
    return true;
  }

  template <typename U>
  static constexpr bool test_getSection(...)
  {
    return false;
  }

  static constexpr bool value = test_getSection<T>(int());
};

/**
 * @brief This is a utility class for loading plugins
 * @details The library_name should not include the prefix 'lib' or suffix '.so'. It will add the correct prefix and
 * suffix based on the OS.
 *
 * It supports providing additional search paths and set environment variable which should be used when searching for
 * plugins.
 *
 * The plugin must be exported using the macro EXPORT_CLASS_SECTIONED.
 * In the example below, the first parameter is the derived object and the second is the assigned symbol name which is
 * used for loading Example: EXPORT_CLASS_SECTIONED(my_namespace::MyPlugin, plugin, section)
 *
 *   PluginLoader loader;
 *   loader.search_libraries.insert("my_plugin"); // libmy_plugin.so
 *   std::shared_ptr<PluginBase> p = loader.instantiate<PluginBase>("plugin");
 */
class PluginLoader
{
public:
  /** @brief Indicate is system folders may be search if plugin is not found in any of the paths */
  bool search_system_folders{ true };

  /** @brief A list of paths to search for plugins */
  std::set<std::string> search_paths;

  /** @brief A list of library names without the prefix or suffix that contain plugins*/
  std::set<std::string> search_libraries;

  /** @brief The environment variable containing plugin search paths */
  std::string search_paths_env;

  /**
   * @brief The environment variable containing plugins
   * @details The plugins are store ins the following format.
   * The library name does not contain prefix or suffix
   *   Format: library_name:library_name1:library_name2
   */
  std::string search_libraries_env;

  /**
   * @brief Loads a shared instance of a plugin of a specified type
   * @throws If the plugin is not found
   * @param plugin_name The plugin name to find
   * @return A shared instance
   */
  template <class PluginBase>
  std::shared_ptr<PluginBase> createInstance(const std::string& plugin_name) const;

  /**
   * @brief Lists all available plugins of a specified base type
   * @details This method requires that each plugin interface definition define a static string member called `section`.
   * This string is used to denote symbols (i.e. plugin classes) in a library, such that all symbols a given section
   * name can be found by the plugin loader. It is useful to specify a unique section name to each plugin interface
   * class in order to find all implementations of that plugin interface in the libraries containing plugins.
   */
  template <class PluginBase>
  typename std::enable_if<has_getSection<PluginBase>::value, std::vector<std::string>>::type
  getAvailablePlugins() const;

  /**
   * @brief Check if plugin is available
   * @param plugin_name The plugin name to find
   * @return True if plugin is found
   */
  inline bool isPluginAvailable(const std::string& plugin_name) const;

  /**
   * @brief Get the available plugins under the provided section
   * @param section The section name to get all available plugins
   * @return A list of available plugins under the provided section
   */
  inline std::vector<std::string> getAvailablePlugins(const std::string& section) const;

  /**
   * @brief Get the available sections within the provided search libraries
   * @return A list of available sections
   */
  inline std::vector<std::string> getAvailableSections(bool include_hidden = false) const;

  /**
   * @brief The number of plugins stored. The size of plugins variable
   * @return The number of plugins.
   */
  inline int count() const;

  /**
   * @brief Check if empty
   * @return True if no search libraries exist
   */
  inline bool empty() const;

protected:
  template <typename PluginBase>
  void reportErrorCommon(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
                         const std::set<std::string>& search_paths,
                         const std::set<std::string>& search_libraries) const;

  template <typename PluginBase>
  typename std::enable_if<!has_getSection<PluginBase>::value, void>::type
  reportError(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
              const std::set<std::string>& search_paths, const std::set<std::string>& search_libraries) const;

  template <typename PluginBase>
  typename std::enable_if<has_getSection<PluginBase>::value, void>::type
  reportError(std::ostream& msg, const std::string& plugin_name, bool search_system_folders,
              const std::set<std::string>& search_paths, const std::set<std::string>& search_libraries) const;
};

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_PLUGIN_LOADER_H
