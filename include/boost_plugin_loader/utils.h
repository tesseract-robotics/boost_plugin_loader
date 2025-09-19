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
#ifndef BOOST_PLUGIN_LOADER_UTILS_H
#define BOOST_PLUGIN_LOADER_UTILS_H

// STD
#include <string>
#include <vector>
#include <optional>

// Boost
#include <boost/dll/shared_library.hpp>

namespace boost_plugin_loader
{

/** @brief The Boost Plugin Loader Exception class */
class PluginLoaderException : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

/**
 * @brief Attempt to load library give library name and directory
 * @param library_name The library name to load which does not include the prefix 'lib' or suffix '.so'
 * @param library_directory The library directory, if empty it will enable search system directories
 * @return A shared library
 */
std::optional<boost::dll::shared_library> loadLibrary(const boost::filesystem::path& library_path);

/**
 * @brief Get a list of available symbols under the provided section
 * @param library The library to search for available symbols
 * @param section The section to search for available symbols
 * @return A list of symbols if they exist.
 */
std::vector<std::string> getAllAvailableSymbols(const boost::dll::shared_library& library, const std::string& section);

/**
 * @brief Get a list of available sections
 * @param library The library to search for available sections
 * @param include_hidden Indicate if hidden sections should be included
 * @return A list of sections if they exist.
 */
std::vector<std::string> getAllAvailableSections(const boost::dll::shared_library& library,
                                                 bool include_hidden = false);

/**
 * @brief Give library name without prefix and suffix it will return the library name with the prefix and suffix
 *
 * * For instance, for a library_name like "boost" it returns :
 * - path/to/libboost.so on posix platforms
 * - path/to/libboost.dylib on OSX
 * - path/to/boost.dll on Windows
 *
 * @todo When support is dropped for 18.04 switch to using boost::dll::shared_library::decorate
 * @param library_name The library name without prefix and suffix
 * @param library_directory The library directory, if empty it just returns the decorated library name
 * @return The library name or path with prefix and suffix
 */
std::string decorate(const std::string& library_name, const std::string& library_directory = "");

/**
 * @brief Extract list form environment variable
 * @details The environment variables should be separated by a colon (":")
 * @param env_variable The environment variable name to extract list from
 * @return A list extracted from variable name
 */
std::vector<std::string> parseEnvironmentVariableList(const std::string& env_variable);

/**
 * @brief Get all available search paths
 * @param search_libraries_env The environment variable containing plugin search paths
 * @param existing_search_libraries A list of existing search paths
 * @return A list of search paths
 */
std::vector<std::string> getAllSearchPaths(const std::string& search_paths_env,
                                           const std::vector<std::string>& existing_search_paths);

/**
 * @brief Get all available library names
 * @param search_libraries_env The environment variable containing plugin library names
 * @param existing_search_libraries A list of existing library names without the prefix or suffix that contain plugins
 * @return A list of library names without the prefix or suffix that contain plugins
 */
std::vector<std::string> getAllLibraryNames(const std::string& search_libraries_env,
                                            const std::vector<std::string>& existing_search_libraries);

/**
 * @brief Utility function to add library containing symbol to the search env variable
 *  * In some cases the name and location of a library is unknown at runtime, but a symbol can
 * be linked at compile time. This is true for Python auditwheel distributions. This
 * utility function will determine the location of the library, and add it to the library search
 * environment variable so it can be found.
 *  * @param symbol_ptr Pointer to the symbol to find
 * @param search_libraries_env The environmental variable to modify
 */
void addSymbolLibraryToSearchLibrariesEnv(const void* symbol_ptr, const std::string& search_libraries_env);

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_UTILS_H
