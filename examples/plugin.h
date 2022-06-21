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
#pragma once

#include <memory>
#include <string>

namespace boost_plugin_loader
{
/** @brief Plugin interface implementation for testing */
class Printer
{
public:
  using Ptr = std::shared_ptr<Printer>;
  virtual void operator()() const = 0;
  static const std::string& getSection();

private:
  friend class PluginLoader;
};

/** @brief Plugin interface implementation for testing */
class Shape
{
public:
  using Ptr = std::shared_ptr<Shape>;
  virtual void operator()() const = 0;
  static const std::string& getSection();

private:
  friend class PluginLoader;
};

}  // namespace boost_plugin_loader

#include <boost_plugin_loader/macros.h>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
#define EXPORT_PRINTER_PLUGIN(DERIVED_CLASS, ALIAS) EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, printer)
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
#define EXPORT_SHAPE_PLUGIN(DERIVED_CLASS, ALIAS) EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, shape)