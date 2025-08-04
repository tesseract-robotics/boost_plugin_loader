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
#include "printer/printer.h"
#include "shape/shape.h"

// STD
#include <string>

// Boost Plugin Loader
#include <boost_plugin_loader/plugin_loader.h>
#include <boost_plugin_loader/plugin_loader.hpp>  // NOLINT(misc-include-cleaner)

// Macros for converting a non-string target compile definition into a string
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)

namespace boost_plugin_loader
{
// Define the section name for loading plugins of base class `Printer`
// This should match the section name specified in the plugin export macro for this class
std::string Printer::getSection()
{
  return STRINGIFY(SECTION_PRINTER);
}
INSTANTIATE_PLUGIN_LOADER(Printer)

// Define the section name for loading plugins of base class `ShapeFactory`
// This should match the section name specified in the plugin export macro for this class
std::string ShapeFactory::getSection()
{
  return STRINGIFY(SECTION_SHAPE);
}
INSTANTIATE_PLUGIN_LOADER(ShapeFactory)

}  // namespace boost_plugin_loader
