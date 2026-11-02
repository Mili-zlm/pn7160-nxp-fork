# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/mili/pn7160-nxp-fork/build/native/native-source")
  file(MAKE_DIRECTORY "/home/mili/pn7160-nxp-fork/build/native/native-source")
endif()
file(MAKE_DIRECTORY
  "/home/mili/pn7160-nxp-fork/build/native/native-build"
  "/home/mili/pn7160-nxp-fork/build/native/native-project"
  "/home/mili/pn7160-nxp-fork/build/native/native-project/tmp"
  "/home/mili/pn7160-nxp-fork/build/native/native-project/src/pn7160-native-stamp"
  "/home/mili/pn7160-nxp-fork/build/native/native-project/src"
  "/home/mili/pn7160-nxp-fork/build/native/native-project/src/pn7160-native-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/mili/pn7160-nxp-fork/build/native/native-project/src/pn7160-native-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/mili/pn7160-nxp-fork/build/native/native-project/src/pn7160-native-stamp${cfgdir}") # cfgdir has leading slash
endif()
