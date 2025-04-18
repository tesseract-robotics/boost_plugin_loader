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
#ifndef BOOST_PLUGIN_LOADER_MACROS_H
#define BOOST_PLUGIN_LOADER_MACROS_H

#include <boost/dll/alias.hpp>

/** @brief Exports a class with an alias name under the "section" namespace */
#define EXPORT_CLASS_SECTIONED(DERIVED_CLASS, ALIAS, SECTION)                                                          \
  extern "C" BOOST_SYMBOL_EXPORT DERIVED_CLASS ALIAS;                                                                  \
  BOOST_DLL_SECTION(SECTION, read) BOOST_DLL_SELECTANY DERIVED_CLASS ALIAS;

#define PLUGIN_ANCHOR_DECL(ANCHOR_NAME) const void* ANCHOR_NAME();  // NOLINT

// clang-format off
#define PLUGIN_ANCHOR_IMPL(ANCHOR_NAME)                                                                                \
  const void* ANCHOR_NAME() { return (const void*)(&ANCHOR_NAME); } // NOLINT
// clang-format on

#endif  // BOOST_PLUGIN_LOADER_MACROS_H
