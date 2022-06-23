# Boost Plugin Loader
Plugin loading library based on Boost DLL

[![codecov](https://codecov.io/gh/tesseract-robotics/boost_plugin_loader/branch/main/graph/badge.svg?token=rTx5ziwNlg)](https://codecov.io/gh/tesseract-robotics/boost_plugin_loader)

Platform             | CI Status
---------------------|:---------
Linux (Focal)        | [![Build Status](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/ubuntu_focal.yml/badge.svg)](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/ubuntu_focal.yml)
Windows              | [![Build Status](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/windows_2019.yml/badge.svg)](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/windows_2019.yml)
Lint  (Clang-Format) | [![Build Status](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/clang_format.yml/badge.svg)](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/clang_format.yml)
Lint  (CMake-Format) | [![Build Status](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/cmake_format.yml/badge.svg)](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/cmake_format.yml)
Lint  (Clang-Tidy)   | [![Build Status](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/ubuntu_focal.yml/badge.svg)](https://github.com/tesseract-robotics/boost_plugin_loader/actions/workflows/ubuntu_focal.yml)

[![Github Issues](https://img.shields.io/github/issues/tesseract-robotics/boost_plugin_loader.svg)](http://github.com/tesseract-robotics/boost_plugin_loader/issues)

[![license - apache 2.0](https://img.shields.io/:license-Apache%202.0-yellowgreen.svg)](https://opensource.org/licenses/Apache-2.0)

## Usage
The plugin loader must know the names of the libraries in which to search for plugins.
These library names should not contain a prefix (i.e., lib/) or suffix (i.e., .so).
The library names can be set in two ways:
1. Set the `search_libraries` member directly in code
1. Add a list of library names to an arbitrary environment variable (separated by colon), and set the `search_libraries_env` member to the name of that environment variable.

The plugin loader must also know the paths in which to look for the specified libraries that contain plugins.
These paths can also be set in three ways:
1. Set the `search_system_folders` member true. This will allow the plugin loader to look for plugins in directories specified by system environment variables (e.g., `LD_LIBRARY_PATH`).
Generally this is the easiest approach.
1. Set the `search_paths` member directly in code
1. Add a list of library names to an arbitrary environment variable (separated by colon), and set the `search_paths_env` member to the name of that environment variable

## Defining a plugin base class
At a minimum, there are no requirements on the definition of a base class that can be used with this plugin loader.
However, there is one optional requirement for enabling the plugin loader to discover the names of all plugins inheriting a specific base class type.
Namely, the plugin base class must have a member function `static std::string getSection()` which defines a section name for the plugin and is accessible to the `PluginLoader` and `has_getSection` classes.
The section name is a unique 8-byte string that associates implementations to the base class.
The plugin loader method `getAvailablePlugins` can identify all symbols in a library with this section name and thereby return all implementations of a particular base class.
It is also generally useful to define a new export macro for the base class that invokes the `EXPORT_CLASS_SECTIONED` macro with the section name directly.
See the [test plugin base class definition](examples/plugin.h) for an example.

## Declaring plugin implementations
Creating an implementation of a plugin is as simple as inheriting from the plugin base class, and calling the `EXPORT_CLASS_SECTIONED` macro with the correct section
(or calling a custom export macro defined for the plugin base class, described above). See the [test plugin implementations](examples/plugin_impl.cpp) for an example.
