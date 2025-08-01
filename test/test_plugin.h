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
#ifndef BOOST_PLUGIN_LOADER_TEST_PLUGIN_H
#define BOOST_PLUGIN_LOADER_TEST_PLUGIN_H

#include <string>

namespace boost_plugin_loader
{
/**
 * @brief Dummy plugin interface for performing multiplication
 */
class TestPluginMultiply
{
public:
  virtual ~TestPluginMultiply() = default;
  virtual double multiply(double x, double y) = 0;
  static std::string getSection()
  {
    return SECTION_MULTIPLY;
  }

protected:
  friend class PluginLoader;
};

/**
 * @brief Dummy plugin interface for performing addition
 */
class TestPluginAdd
{
public:
  virtual ~TestPluginAdd() = default;
  virtual double add(double x, double y) = 0;
  static std::string getSection()
  {
    return SECTION_ADD;
  }

protected:
  friend class PluginLoader;
};

}  // namespace boost_plugin_loader

// Macros for converting a non-string target compile definition into a string
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

/**
 * @brief Function for getting the symbol name from the target compile definition __SYMBOL_NAME__
 */
static inline std::string getSymbolName()
{
  return STRINGIFY(__SYMBOL_NAME__);
}

// Create macros to export both plugins with a given alias and with section names defined as target compile definitions
#include <boost_plugin_loader/macros.h>
#define EXPORT_TEST_PLUGIN_MULTIPLY(DERIVED_CLASS, ALIAS) EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, SECTION_MULTIPLY)
#define EXPORT_TEST_PLUGIN_ADD(DERIVED_CLASS, ALIAS) EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, SECTION_ADD)

#endif  // BOOST_PLUGIN_LOADER_TEST_PLUGIN_H
