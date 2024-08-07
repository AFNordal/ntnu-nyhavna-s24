# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/include/pico-sdk/tools/pioasm"
  "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pioasm"
  "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pico-sdk/src/rp2_common/tinyusb/pioasm"
  "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pico-sdk/src/rp2_common/tinyusb/pioasm/tmp"
  "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
  "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src"
  "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspaces/ntnu-nyhavna-s24/tracking-box/sd-test/build/pico-sdk/src/rp2_common/tinyusb/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
