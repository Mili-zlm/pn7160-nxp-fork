# Install script for directory: /home/mili/pn7160-nxp-fork

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/mili/pn7160-nxp-fork/build/fixes-multi/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        file(RPATH_CHECK
             FILE "${file}"
             RPATH "")
      endif()
    endforeach()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug/libnfc_nci_linux-1.so.0.0.0"
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug/libnfc_nci_linux-1.so.0"
      )
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        if(CMAKE_INSTALL_DO_STRIP)
          execute_process(COMMAND "/usr/bin/strip" "${file}")
        endif()
      endif()
    endforeach()
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        file(RPATH_CHECK
             FILE "${file}"
             RPATH "")
      endif()
    endforeach()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release/libnfc_nci_linux-1.so.0.0.0"
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release/libnfc_nci_linux-1.so.0"
      )
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        if(CMAKE_INSTALL_DO_STRIP)
          execute_process(COMMAND "/usr/bin/strip" "${file}")
        endif()
      endif()
    endforeach()
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        file(RPATH_CHECK
             FILE "${file}"
             RPATH "")
      endif()
    endforeach()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo/libnfc_nci_linux-1.so.0.0.0"
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo/libnfc_nci_linux-1.so.0"
      )
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libnfc_nci_linux-1.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        if(CMAKE_INSTALL_DO_STRIP)
          execute_process(COMMAND "/usr/bin/strip" "${file}")
        endif()
      endif()
    endforeach()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug/libnfc_nci_linux-1.so")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release/libnfc_nci_linux-1.so")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo/libnfc_nci_linux-1.so")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        file(RPATH_CHECK
             FILE "${file}"
             RPATH "")
      endif()
    endforeach()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug/libpn7160_fw.so.0.0.0"
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug/libpn7160_fw.so.0"
      )
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        if(CMAKE_INSTALL_DO_STRIP)
          execute_process(COMMAND "/usr/bin/strip" "${file}")
        endif()
      endif()
    endforeach()
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        file(RPATH_CHECK
             FILE "${file}"
             RPATH "")
      endif()
    endforeach()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release/libpn7160_fw.so.0.0.0"
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release/libpn7160_fw.so.0"
      )
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        if(CMAKE_INSTALL_DO_STRIP)
          execute_process(COMMAND "/usr/bin/strip" "${file}")
        endif()
      endif()
    endforeach()
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        file(RPATH_CHECK
             FILE "${file}"
             RPATH "")
      endif()
    endforeach()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo/libpn7160_fw.so.0.0.0"
      "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo/libpn7160_fw.so.0"
      )
    foreach(file
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0.0.0"
        "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libpn7160_fw.so.0"
        )
      if(EXISTS "${file}" AND
         NOT IS_SYMLINK "${file}")
        if(CMAKE_INSTALL_DO_STRIP)
          execute_process(COMMAND "/usr/bin/strip" "${file}")
        endif()
      endif()
    endforeach()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug/libpn7160_fw.so")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release/libpn7160_fw.so")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo/libpn7160_fw.so")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      file(RPATH_CHECK
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp"
           RPATH [[$ORIGIN/../../lib]])
    endif()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/apps/tools" TYPE EXECUTABLE FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/Debug/nfcDemoApp")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      file(RPATH_CHANGE
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp"
           OLD_RPATH "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug:"
           NEW_RPATH [[$ORIGIN/../../lib]])
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      endif()
    endif()
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      file(RPATH_CHECK
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp"
           RPATH [[$ORIGIN/../../lib]])
    endif()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/apps/tools" TYPE EXECUTABLE FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/Release/nfcDemoApp")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      file(RPATH_CHANGE
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp"
           OLD_RPATH "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release:"
           NEW_RPATH [[$ORIGIN/../../lib]])
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      endif()
    endif()
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      file(RPATH_CHECK
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp"
           RPATH [[$ORIGIN/../../lib]])
    endif()
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/apps/tools" TYPE EXECUTABLE FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/RelWithDebInfo/nfcDemoApp")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      file(RPATH_CHANGE
           FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp"
           OLD_RPATH "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo:"
           NEW_RPATH [[$ORIGIN/../../lib]])
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/apps/tools/nfcDemoApp")
      endif()
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Debug/libnfc_nci_linux.so")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/Release/libnfc_nci_linux.so")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/custom-lib/RelWithDebInfo/libnfc_nci_linux.so")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/mili/pn7160-nxp-fork/src/include/linux_nfc_api.h"
    "/home/mili/pn7160-nxp-fork/src/include/linux_nfc_factory_api.h"
    "/home/mili/pn7160-nxp-fork/src/include/linux_nfc_api_compatibility.h"
    "/home/mili/pn7160-nxp-fork/src/include/pn7160_board_config.h"
    "/home/mili/pn7160-nxp-fork/src/include/pn7160_diag.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/etc" TYPE FILE FILES
    "/home/mili/pn7160-nxp-fork/conf/libnfc-nci.conf"
    "/home/mili/pn7160-nxp-fork/conf/libnfc-nxp.conf"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/mili/pn7160-nxp-fork/build/fixes-multi/libnfc-nci.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/mili/pn7160-nxp-fork/build/fixes-multi/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/mili/pn7160-nxp-fork/build/fixes-multi/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
