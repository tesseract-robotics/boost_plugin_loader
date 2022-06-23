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
#ifndef BOOST_PLUGIN_LOADER_TEST_PLUGIN_BASE_H
#define BOOST_PLUGIN_LOADER_TEST_PLUGIN_BASE_H

#include <string>

namespace boost_plugin_loader
{
class TestPluginBase
{
public:
  virtual ~TestPluginBase() = default;
  virtual double multiply(double x, double y) = 0;
  static std::string getSection()
  {
    return "TestBase";
  }

protected:
  friend class PluginLoader;
};

}  // namespace boost_plugin_loader

#include <boost_plugin_loader/macros.h>
#define EXPORT_TEST_PLUGIN(DERIVED_CLASS, ALIAS) EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, TestBase)

#endif  // BOOST_PLUGIN_LOADER_TEST_PLUGIN_BASE_H
