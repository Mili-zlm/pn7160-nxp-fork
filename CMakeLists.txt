cmake_minimum_required(VERSION 3.16)
project(PN7160Native LANGUAGES C CXX)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "This NXP backend requires Linux (i2c-dev, sysfs GPIO and POSIX); other OS backends are not implemented")
endif()
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Use an out-of-source build: cmake -S . -B build/native")
endif()
include(CheckCXXSourceCompiles)
find_package(Threads REQUIRED)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "Only 32/64-bit target ABIs are supported")
endif()
math(EXPR PN7160_TARGET_BITS "${CMAKE_SIZEOF_VOID_P} * 8")
set(_abi "${CMAKE_C_COMPILER}|${CMAKE_CXX_COMPILER}|${PN7160_TARGET_BITS}|${CMAKE_SYSTEM_PROCESSOR}|${CMAKE_SYSROOT}")
if(EXISTS "${CMAKE_BINARY_DIR}/target-abi.txt")
  file(READ "${CMAKE_BINARY_DIR}/target-abi.txt" _old_abi)
  if(NOT _old_abi STREQUAL _abi)
    message(FATAL_ERROR "Target ABI/toolchain changed; use a new build directory")
  endif()
endif()
file(WRITE "${CMAKE_BINARY_DIR}/target-abi.txt" "${_abi}")
include(GNUInstallDirs)
set(PN7160_DEFAULT_I2C_DEVICE "/dev/i2c-1" CACHE STRING "Absolute I2C device path")
set(PN7160_DEFAULT_I2C_ADDRESS "0x28" CACHE STRING "Unshifted 7-bit address")
set(PN7160_DEFAULT_IRQ_GPIO "23" CACHE STRING "Linux sysfs global IRQ number")
set(PN7160_DEFAULT_VEN_GPIO "24" CACHE STRING "Linux sysfs global VEN number")
set(PN7160_DEFAULT_DWL_REQ_GPIO "-1" CACHE STRING "Optional sysfs DWL number; -1 disables")
set(PN7160_DEFAULT_RESET_DELAY_US "10000" CACHE STRING "Reset delay in microseconds")
if(NOT PN7160_DEFAULT_I2C_DEVICE MATCHES "^/[A-Za-z0-9_./-]+$")
  message(FATAL_ERROR "I2C device must be an absolute path without whitespace or C escape characters")
endif()
foreach(setting I2C_ADDRESS IRQ_GPIO VEN_GPIO DWL_REQ_GPIO RESET_DELAY_US)
  if(NOT "${PN7160_DEFAULT_${setting}}" MATCHES "^(-?[0-9]+|0[xX][0-9A-Fa-f]+)$")
    message(FATAL_ERROR "Invalid numeric setting PN7160_DEFAULT_${setting}")
  endif()
endforeach()
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")
configure_file(cmake/board_defaults.h.in generated/pn7160_board_build_config.h @ONLY)
file(WRITE "${CMAKE_BINARY_DIR}/check-board-defaults.cpp"
  "#include \"pn7160_board_config.h\"\nint main(){pn7160_config_t c;return pn7160_get_default_config(&c,sizeof(c));}\n")
try_compile(PN7160_DEFAULTS_VALID "${CMAKE_BINARY_DIR}/check-defaults"
  SOURCES "${CMAKE_SOURCE_DIR}/src/pn7160_board_config.cpp"
          "${CMAKE_BINARY_DIR}/check-board-defaults.cpp"
  CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=11"
    "-DINCLUDE_DIRECTORIES=${CMAKE_SOURCE_DIR}/src/include;${CMAKE_BINARY_DIR}/generated"
  COMPILE_DEFINITIONS -DPN7160_USE_BUILD_DEFAULTS
  LINK_LIBRARIES Threads::Threads
  OUTPUT_VARIABLE PN7160_DEFAULTS_LOG)
if(NOT PN7160_DEFAULTS_VALID)
  message(FATAL_ERROR "Invalid board defaults or unsupported C++ toolchain: ${PN7160_DEFAULTS_LOG}")
endif()
check_cxx_source_compiles("#include <atomic>\n#include <cstdint>\nstd::atomic<uint64_t> x(0); int main(){x.fetch_add(1);return (int)x.load();}" PN7160_ATOMIC_NATIVE)
set(PN7160_ATOMIC_LIBS "")
if(NOT PN7160_ATOMIC_NATIVE)
  set(CMAKE_REQUIRED_LIBRARIES atomic)
  check_cxx_source_compiles("#include <atomic>\n#include <cstdint>\nstd::atomic<uint64_t> x(0); int main(){x.fetch_add(1);return (int)x.load();}" PN7160_ATOMIC_LIBRARY)
  unset(CMAKE_REQUIRED_LIBRARIES)
  if(NOT PN7160_ATOMIC_LIBRARY)
    message(FATAL_ERROR "Target toolchain cannot link 64-bit atomics, even with libatomic")
  endif()
  set(PN7160_ATOMIC_LIBS "-latomic")
endif()
check_cxx_source_compiles("static_assert(sizeof(void*) == ${CMAKE_SIZEOF_VOID_P}, \"ABI mismatch\"); int main(){return 0;}" PN7160_C_CPP_ABI_MATCH)
if(NOT PN7160_C_CPP_ABI_MATCH)
  message(FATAL_ERROR "C/C++ target ABIs differ")
endif()

# Explicit source manifest: CMake never reads Makefile.am at configure/build time.
add_library(nfc_nci_linux SHARED
  src/android/utility/base/file.cpp
  src/libnfc-utils/src/ConfigPathProvider.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/rw/nfa_rw_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/rw/nfa_rw_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/rw/nfa_rw_act.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_mfc.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/hci/nfa_hci_ci.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/hci/nfa_hci_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/hci/nfa_hci_act.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/hci/nfa_hci_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/hci/nfa_hci_utils.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/ce/nfa_ce_act.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/ce/nfa_ce_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/ce/nfa_ce_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/p2p/nfa_p2p_act.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/p2p/nfa_p2p_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/p2p/nfa_p2p_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/sys/nfa_sys_cback.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/sys/nfa_sys_ptim.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/sys/nfa_sys_cfg.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/sys/nfa_sys_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/dm/nfa_dm_discover.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/dm/nfa_dm_act.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/dm/nfa_dm_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/dm/nfa_dm_ndef.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/dm/nfa_dm_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/dm/nfa_dm_cfg.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/ee/nfa_ee_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/ee/nfa_ee_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/ee/nfa_ee_act.cc
  src/nxp_nci_hal_libnfc-nci/src/adaptation/libmain.cc
  src/nxp_nci_hal_libnfc-nci/src/gki/common/gki_debug.cc
  src/nxp_nci_hal_libnfc-nci/src/gki/common/gki_time.cc
  src/nxp_nci_hal_libnfc-nci/src/gki/common/gki_buffer.cc
  src/nxp_nci_hal_libnfc-nci/src/gki/ulinux/gki_ulinux.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/llcp/llcp_sdp.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/llcp/llcp_link.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/llcp/llcp_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/llcp/llcp_util.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/llcp/llcp_dlc.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/llcp/llcp_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nfc/nfc_vs.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nfc/nfc_task.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nfc/nfc_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nfc/nfc_ncif.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nfc/nfc_test.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nfc/nfc_ee.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nfc/nfc_utils.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nci/nci_hrcv.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/nci/nci_hmsgs.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_i93.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_t4t.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/tags_int.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/ce_t3t.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_t2t.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_t2t_ndef.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_t1t.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/ce_t4t.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_t3t.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/ce_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/tags/rw_t1t_ndef.cc
  src/nxp_nci_hal_libnfc-nci/src/nfc/ndef/ndef_utils.cc
  src/nxp_nci_hal_libnfc-nci/utils/config.cc
  src/nxp_nci_hal_libnfc-nci/src/adaptation/CrcChecksum.cc
  src/nxp_nci_hal_libnfc-nci/src/adaptation/NfcAdaptation.cc
  src/libnfc-utils/src/android_logmsg.cpp
  src/nxp_nci_hal_libnfc-nci/src/adaptation/nfc_config.cc
  src/nxp_nci_hal_libnfc-nci/utils/ringbuffer.cc
  src/libnfc-utils/cho/ndef_cho_utils.cc
  src/libnfc-utils/src/OverrideLog.cpp
  src/android/utility/base/phNxpLogging.cpp
  src/android/utility/base/strings.cpp
  src/android/utility/android-base/stringprintf.cpp
  src/nxp_nci_hal_libnfc-nci/src/nfa/t4tnfcee/nfa_nfcee_main.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/t4tnfcee/nfa_nfcee_api.cc
  src/nxp_nci_hal_libnfc-nci/src/nfa/t4tnfcee/nfa_nfcee_act.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/phDal4Nfc_messageQueueLib.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/phOsalNfc_Timer.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/phTmlNfc.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/dnld/phDnldNfc.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/dnld/phNxpNciHal_Dnld.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/dnld/phDnldNfc_Internal.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/dnld/phDnldNfc_Utils.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/log/phNxpLog.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/self-test/phNxpNciHal_SelfTest.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/hal/phNxpNciHal_NfcDepSWPrio.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/hal/phNxpNciHal_dta.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/hal/phNxpNciHal_ext.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/hal/phNxpNciHal.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/utils/phNxpNciHal_utils.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/utils/phNxpConfig.cpp
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/NfccTransportFactory.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/utils/NfccPowerTracker.cpp
  src/nfcandroid_nfc_hidlimpl/halimpl/mifare/NxpMfcReader.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/utils/NxpNfcCapability.cpp
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/transport/NfccI2cTransport.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/transport/NfccAltI2cTransport.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/transport/NfccAltSpiTransport.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/transport/NfccAltTransport.cc
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/transport/NfccTransport.cc
  src/nxp_nci_hal_nfc/nci/jni/Mutex.cpp
  src/nxp_nci_hal_nfc/nci/jni/IntervalTimer.cpp
  src/nxp_nci_hal_nfc/nci/jni/CondVar.cpp
  src/nxp_nci_hal_nfc/nci/jni/NativeNfcTag.cpp
  src/nxp_nci_hal_nfc/nci/jni/NfcTag.cpp
  src/nxp_nci_hal_nfc/nci/jni/NativeNxpFeature.cpp
  src/nxp_nci_hal_nfc/nci/jni/NativeT4tNfcee.cpp
  src/nxp_nci_hal_nfc/nci/jni/NativeT4tNfceeManager.cpp
  src/service/interface/nativeNdef.cpp
  src/nxp_nci_hal_nfc/nci/jni/RoutingManager.cpp
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/src/mifare/phFriNfc_SmtCrdFmt.cpp
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/src/mifare/phNxpExtns_MifareStd.cpp
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/src/mifare/phFriNfc_MifStdFormat.cpp
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/src/mifare/phFriNfc_MifareStdMap.cpp
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/src/phNxpExtns.cpp
  src/service/linux_nfc_api.cpp
  src/service/linux_nfc_factory_api.cpp
  src/service/interface/nativeNfcSnep.cpp
  src/libnfc-utils/src/nfa_snep_api.cpp
  src/libnfc-utils/src/nfa_snep_act.cc
  src/libnfc-utils/src/nfa_snep_main.cc
  src/libnfc-utils/src/nfa_snep_default.cc
  src/service/interface/nativeNfcHandover.cpp
  src/service/interface/nativeNfcLlcp.cpp
  src/nxp_nci_hal_nfc/nci/jni/NativeNfcManager.cpp
  src/nxp_nci_hal_nfc/nci/jni/PeerToPeer.cpp
  src/pn7160_board_config.cpp
)
target_include_directories(nfc_nci_linux
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated" firmware/pn7160
  src/nxp_nci_hal_libnfc-nci/src/include
  src/nxp_nci_hal_libnfc-nci/src/
  src/nxp_nci_hal_libnfc-nci/src/gki/ulinux
  src/nxp_nci_hal_libnfc-nci/src/gki/common
  src/nxp_nci_hal_libnfc-nci/src/nfa/include
  src/nxp_nci_hal_libnfc-nci/src/nfa/int
  src/nxp_nci_hal_libnfc-nci/src/nfc/include
  src/nxp_nci_hal_libnfc-nci/src/nfc/int
  src/nxp_nci_hal_libnfc-nci/src/hal/include
  src/nxp_nci_hal_libnfc-nci/src/hal/int
  src/nxp_nci_hal_libnfc-nci/src/fuzzers/inc/android/hardware/nfc/1.1/
  src/android/utility/
  src/nfcandroid_nfc_hidlimpl/halimpl/utils/
  src/libnfc-utils/inc
  src/nfcandroid_nfc_hidlimpl/halimpl/common/
  src/android/utility/hardware/
  src/android/utility/android/
  src/nfcandroid_nfc_hidlimpl/halimpl/inc/
  src/nfcandroid_nfc_hidlimpl/halimpl/dnld/
  src/nfcandroid_nfc_hidlimpl/halimpl/hal/
  src/nfcandroid_nfc_hidlimpl/halimpl/log/
  src/nfcandroid_nfc_hidlimpl/halimpl/tml
  src/nxp_nci_hal_libnfc-nci/src/fuzzers/inc/android/hardware
  src/nfcandroid_nfc_hidlimpl/halimpl/self-test
  src/nfcandroid_nfc_hidlimpl/halimpl/mifare
  src/nfcandroid_nfc_hidlimpl/halimpl/tml/transport
  src/nfcandroid_nfc_hidlimpl/halimpl/src/include
  src/nxp_nci_hal_libnfc-nci/src/fuzzers/inc/
  src/nxp_nci_hal_libnfc-nci/utils/include
  src/nxp_nci_hal_libnfc-nci/utils
  src/nfcandroid_nfc_hidlimpl/extns/impl/
  src/android/secure_element/extns/impl/
  src/android/utility/base/
  src/service/interface
  src/service/utils
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/inc
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/src/common
  src/nxp_nci_hal_nfc/nci/jni/extns/pn54x/src/mifare
  src/nxp_nci_hal_nfc/nci/jni/
  src/nfcandroid_nfc_hidlimpl/halimpl/log
)
target_compile_features(nfc_nci_linux PRIVATE cxx_std_11)
target_compile_options(nfc_nci_linux PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-include;cstdint>)
target_compile_definitions(nfc_nci_linux PRIVATE
  CONFIG_PATH="${CMAKE_INSTALL_FULL_SYSCONFDIR}/"
  PN7160_USE_BUILD_DEFAULTS NXP_EXTNS=TRUE LINUX SNEP_ENABLED
  NFC_NXP_NOT_OPEN_INCLUDED=TRUE NXP_HW_SELF_TEST NXP_NFC_NATIVE_ENABLE_HCE=TRUE
  PH_NCI_NXP_HAL_ENABLE_FW_DOWNLOAD=FALSE NXP_NEW_MW_API_COMPATIBILITY
  $<$<CONFIG:Debug>:DEBUG>)
option(PN7160_ENABLE_LLCP13 "Enable secured LLCP 1.3 (requires compatible OpenSSL)" OFF)
if(PN7160_ENABLE_LLCP13)
  find_package(OpenSSL REQUIRED)
  target_compile_definitions(nfc_nci_linux PRIVATE NFC_NXP_LLCP_SECURED_P2P=TRUE)
  target_link_libraries(nfc_nci_linux PRIVATE OpenSSL::SSL OpenSSL::Crypto)
else()
  target_compile_definitions(nfc_nci_linux PRIVATE NFC_NXP_LLCP_SECURED_P2P=FALSE)
endif()
target_link_libraries(nfc_nci_linux PRIVATE Threads::Threads ${CMAKE_DL_LIBS} rt)
if(PN7160_ATOMIC_LIBS)
  target_link_libraries(nfc_nci_linux PRIVATE atomic)
endif()
# Preserve the SONAME and filenames used by existing NXP/Python clients.
set_target_properties(nfc_nci_linux PROPERTIES OUTPUT_NAME nfc_nci_linux-1
  VERSION 0.0.0 SOVERSION 0 POSITION_INDEPENDENT_CODE ON)
add_custom_command(TARGET nfc_nci_linux POST_BUILD
  BYPRODUCTS libnfc_nci_linux.so
  COMMAND "${CMAKE_COMMAND}" -E create_symlink "$<TARGET_FILE_NAME:nfc_nci_linux>"
    "${CMAKE_CURRENT_BINARY_DIR}/libnfc_nci_linux.so" VERBATIM)

add_library(pn7160_fw SHARED firmware/pn7160/phDnldNfc_UpdateSeq.cc firmware/pn7160/phDnldNfc_ImageInfo.cc)
target_include_directories(pn7160_fw PRIVATE firmware/pn7160 src/nfcandroid_nfc_hidlimpl/halimpl/common src/nxp_nci_hal_libnfc-nci/src/include)
target_compile_definitions(pn7160_fw PRIVATE LINUX NXP_EXTNS=TRUE)
set_target_properties(pn7160_fw PROPERTIES VERSION 0.0.0 SOVERSION 0)
add_executable(nfcDemoApp demoapp/main.c demoapp/tools.c)
target_include_directories(nfcDemoApp PRIVATE demoapp)
target_link_libraries(nfcDemoApp PRIVATE nfc_nci_linux Threads::Threads ${CMAKE_DL_LIBS} rt)
set_target_properties(nfcDemoApp PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

install(TARGETS nfc_nci_linux pn7160_fw nfcDemoApp
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libnfc_nci_linux.so" DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES src/include/linux_nfc_api.h src/include/linux_nfc_factory_api.h
  src/include/linux_nfc_api_compatibility.h src/include/pn7160_board_config.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES conf/libnfc-nci.conf conf/libnfc-nxp.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR})
configure_file(cmake/libnfc-nci.pc.in libnfc-nci.pc @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libnfc-nci.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
message(STATUS "PN7160 native Linux ${PN7160_TARGET_BITS}-bit; config=${CMAKE_INSTALL_FULL_SYSCONFDIR}")
