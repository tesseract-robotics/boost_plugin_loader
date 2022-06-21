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
#include "plugin.h"
#include <boost_plugin_loader/plugin_loader.h>
#include <iostream>

using boost_plugin_loader::PluginLoader;
using boost_plugin_loader::Printer;
using boost_plugin_loader::Shape;

int main(int /*argc*/, char** /*argv*/)  // NOLINT
{
  PluginLoader loader;
  loader.search_paths.insert(PLUGIN_DIR);
  loader.search_libraries.insert(PLUGINS);

  std::vector<std::string> printer_plugins = loader.getAvailablePlugins<Printer>();
  assert(printer_plugins.size() == 2);

  for (const std::string& plugin_name : printer_plugins)
  {
    std::cout << "Loading plugin '" << plugin_name << "'" << std::endl;
    Printer::Ptr plugin = loader.createInstance<Printer>(plugin_name);
    assert(plugin != nullptr);
    plugin->operator()();
  }

  std::vector<std::string> shape_plugins = loader.getAvailablePlugins<Shape>();
  assert(shape_plugins.size() == 2);

  for (const std::string& plugin_name : shape_plugins)
  {
    std::cout << "Loading plugin '" << plugin_name << "'" << std::endl;
    Printer::Ptr plugin = loader.createInstance<Printer>(plugin_name);
    assert(plugin != nullptr);
    plugin->operator()();
  }
}
