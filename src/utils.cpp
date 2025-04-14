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

// Boost
#include <boost/dll/library_info.hpp>
#include <boost/dll/shared_library.hpp>
#include <boost/dll/shared_library_load_mode.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/error_code.hpp>

// STD
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <optional>
#include <cstring>
#include <cstdlib>

// Boost Plugin Loader
#include <boost_plugin_loader/utils.h>

namespace boost_plugin_loader
{
std::optional<boost::dll::shared_library> loadLibrary(const boost::filesystem::path& library_path)
{
  boost::dll::load_mode::type mode;

  if (!library_path.has_parent_path())
  {
    mode = boost::dll::load_mode::append_decorations | boost::dll::load_mode::search_system_folders;
  }
  else
  {
    mode = boost::dll::load_mode::append_decorations;
  }

  boost::system::error_code ec;
  boost::dll::shared_library lib = boost::dll::shared_library(library_path, ec, mode);
  if (ec)
    return std::nullopt;

  return lib;
}

std::vector<std::string> getAllAvailableSymbols(const std::string& section, const boost::dll::shared_library& library)
{
  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(library.location());

  // Getting symbols exported from he provided section
  return inf.symbols(section);
}

std::vector<std::string> getAllAvailableSections(const boost::dll::shared_library& library, bool include_hidden)
{
  // Class `library_info` can extract information from a library
  boost::dll::library_info inf(library.location());

  // Getting section from library
  std::vector<std::string> sections = inf.sections();

  auto search_fn = [include_hidden](const std::string& section) {
    if (section.empty())
      return true;

    if (include_hidden)
      return false;

    return (section.substr(0, 1) == ".");
  };

  sections.erase(std::remove_if(sections.begin(), sections.end(), search_fn), sections.end());
  return sections;
}

std::string decorate(const std::string& library_name, const std::string& library_directory)
{
  boost::filesystem::path lib_path;
  if (library_directory.empty())
    lib_path = boost::filesystem::path(library_name);
  else
    lib_path = boost::filesystem::path(library_directory) / library_name;

  boost::filesystem::path actual_path =
      (std::strncmp(lib_path.filename().string().c_str(), "lib", 3) != 0 ?
           boost::filesystem::path((lib_path.has_parent_path() ? lib_path.parent_path() / L"lib" : L"lib").native() +
                                   lib_path.filename().native()) :
           lib_path);

  actual_path += boost::dll::shared_library::suffix();
  return actual_path.string();
}

std::set<std::string> parseEnvironmentVariableList(const std::string& env_variable)
{
  std::set<std::string> list;
  char* env_var = std::getenv(env_variable.c_str());
  if (env_var == nullptr)  // Environment variable not found
    return list;

  std::string evn_str = std::string(env_var);
#ifndef _WIN32
  boost::split(list, evn_str, boost::is_any_of(":"), boost::token_compress_on);
#else
  boost::split(list, evn_str, boost::is_any_of(";"), boost::token_compress_on);
#endif
  return list;
}

std::set<std::string> getAllSearchPaths(const std::string& search_paths_env,
                                        const std::set<std::string>& existing_search_paths)
{
  // Check for environment variable to override default library
  if (!search_paths_env.empty())
  {
    std::set<std::string> search_paths = parseEnvironmentVariableList(search_paths_env);
    search_paths.insert(existing_search_paths.begin(), existing_search_paths.end());
    return search_paths;
  }

  return existing_search_paths;
}

std::set<std::string> getAllLibraryNames(const std::string& search_libraries_env,
                                         const std::set<std::string>& existing_search_libraries)
{
  // Check for environment variable to override default library
  if (!search_libraries_env.empty())
  {
    std::set<std::string> search_libraries = parseEnvironmentVariableList(search_libraries_env);
    search_libraries.insert(existing_search_libraries.begin(), existing_search_libraries.end());
    return search_libraries;
  }

  return existing_search_libraries;
}

}  // namespace boost_plugin_loader
