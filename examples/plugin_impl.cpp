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

#include <iostream>

namespace boost_plugin_loader
{
struct ConsolePrinter : public Printer
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: ConsolePrinter" << std::endl;
  }
};

struct HelloWorldPrinter : public Printer
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: Hello World" << std::endl;
  }
};

struct Square : public Shape
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: Square" << std::endl;
  }
};

struct Triangle : public Shape
{
public:
  void operator()() const override
  {
    std::cout << "IMPL: Triangle" << std::endl;
  }
};

}  // namespace boost_plugin_loader

EXPORT_PRINTER_PLUGIN(boost_plugin_loader::ConsolePrinter, ConsolePrinter)
EXPORT_PRINTER_PLUGIN(boost_plugin_loader::HelloWorldPrinter, HelloWorldPrinter)
EXPORT_SHAPE_PLUGIN(boost_plugin_loader::Square, Square)
EXPORT_SHAPE_PLUGIN(boost_plugin_loader::Triangle, Triangle)
