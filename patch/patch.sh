#!/bin/bash
patch_dir=$(dirname $0)
set -e
echo "Setting up esp8266 development Arduino SDK..."
commit_sha1=ab7e109e4c1e30c190279cc877601789c62501ca
framework_dir=${patch_dir}/framework-arduinoespressif8266
target_dir=${HOME}/.platformio/packages/framework-arduinoespressif8266
if [ ! -d "${framework_dir}" ]; then
  git clone https://github.com/esp8266/Arduino ${framework_dir}
fi
(cd ${framework_dir} && git fetch && git reset --hard ${commit_sha1})

patch_bootloaders_dir=${framework_dir}/bootloaders/
patch_cores_dir=${framework_dir}/cores/
patch_libraries=${framework_dir}/libraries/
patch_tools=${framework_dir}/tools/
patch_boards=${framework_dir}/boards.txt

target_bootloaders_dir=${target_dir}/bootloaders
target_cores_dir=${target_dir}/cores
target_libraries=${target_dir}/libraries
target_tools=${target_dir}/tools
target_boards=${target_dir}/boards.txt
cp -R -f ${patch_libraries}/SD/src/* ${patch_libraries}/ESP8266WiFi/src/

if [ -d "${target_dir}" ];
then
  echo "Copying new framework to ${target_dir}/"
  rm -rf ${target_bootloaders_dir}
  rm -rf ${target_cores_dir}
  rm -rf ${target_libraries}
  rm -rf ${target_tools}

  cp -R -f ${patch_bootloaders_dir} ${target_bootloaders_dir}
  cp -R -f ${patch_cores_dir} ${target_cores_dir}
  cp -R -f ${patch_libraries} ${target_libraries}
  cp -R -f ${patch_boards} ${target_boards}
  cp -R -f ${patch_tools} ${target_tools}

  echo "Done."
else
  echo "Could not find ${target_dir}, bailing out"
  exit 1
fi
