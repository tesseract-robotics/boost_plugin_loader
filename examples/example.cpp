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
#include <boost_plugin_loader/plugin_loader.h>

// Include the plugin APIs
#include "printer/printer.h"
#include "shape/shape.h"

#include <iomanip>
#include <iostream>
#include <cassert>

using boost_plugin_loader::PluginLoader;
using boost_plugin_loader::Printer;
using boost_plugin_loader::Shape;
using boost_plugin_loader::ShapeFactory;

void demoPrinterPlugins(const PluginLoader& loader)
{
  std::cout << "Loading printer plugins" << std::endl;

  std::vector<std::string> printer_plugins = loader.getAvailablePlugins<Printer>();
  assert(printer_plugins.size() == 2);

  for (const std::string& plugin_name : printer_plugins)
  {
    std::cout << "Loading plugin '" << plugin_name << "'" << std::endl;
    Printer::Ptr plugin = loader.createInstance<Printer>(plugin_name);
    assert(plugin != nullptr);
    plugin->operator()();

    // Note: `plugin` goes out of scope here, and the library providing its definition will be unloaded
  }
}

void demoShapePlugins(const PluginLoader& loader)
{
  std::cout << "Loading shape plugins" << std::endl;

  std::vector<std::string> shape_plugins = loader.getAvailablePlugins<ShapeFactory>();
  assert(shape_plugins.size() == 2);

  // Create the square and triangle factory plugins
  // Note: these factories must stay in scope as long as they and any object they create are being used. Otherwise, the
  // library providing them will be unloaded, resulting in undefined behavior and potential segfaults
  auto square_factory = loader.createInstance<ShapeFactory>("Square");
  auto triangle_factory = loader.createInstance<ShapeFactory>("Triangle");

  // Create two different types of squares
  Shape::Ptr square_1 = square_factory->create(2.0);
  Shape::Ptr square_2 = square_factory->create(4.0);

  // Create two different types of triangles
  Shape::Ptr triangle_1 = triangle_factory->create(std::tuple<double, double>{ 2.0, 3.0 });
  Shape::Ptr triangle_2 = triangle_factory->create(std::tuple<double, double>{ 8.0, 20.0 });

  // Compute the areas of all of the different shapes
  std::cout << std::setprecision(2) << "Square 1 area:   " << square_1->area() << std::endl;
  std::cout << std::setprecision(2) << "Square 2 area:   " << square_2->area() << std::endl;
  std::cout << std::setprecision(2) << "Triangle 1 area: " << triangle_1->area() << std::endl;
  std::cout << std::setprecision(2) << "Triangle 2 area: " << triangle_2->area() << std::endl;
}

int main(int /*argc*/, char** /*argv*/)  // NOLINT
{
  // Create and configure the plugin loader to be able to find libraries in which plugins exist
  PluginLoader loader;
  loader.search_paths.insert(PLUGIN_DIR);
  loader.search_libraries.insert(PLUGINS);

  // Printer plugins
  demoPrinterPlugins(loader);

  // Shape plugins
  std::cout << std::endl;
  demoShapePlugins(loader);
}
