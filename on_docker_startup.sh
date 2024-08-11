#!/bin/bash
echo `basename "$0"`

cd $WEST_WORKSPACE_CONTAINER

WEST_CONFIG_DIR=$WEST_WORKSPACE_CONTAINER/.west

if [ -d "$WEST_CONFIG_DIR" ];
then
    echo "West already initialized"
else
    west init
fi
# west config needs relativ path, thus basename
west config manifest.path $(basename $WORKDIR_CONTAINER)
west update
west zephyr-export

. $ZEPHRY_BOX_PYTHON_VENV/bin/activate
# Zephyr and modules
pip3 install -r zephyr/scripts/requirements.txt
pip3 install -r bootloader/mcuboot/scripts/requirements.txt
pip3 install -r nrf/scripts/requirements.txt

cd $WORKDIR_CONTAINER

cd hka-embedded-iot

echo ". $ZEPHRY_BOX_PYTHON_VENV/bin/activate" >> ~/.bashrc
echo "export ZEPHYR_BASE=$WEST_WORKSPACE_CONTAINER/zephyr" >> ~/.bashrc
echo "export ZEPHRY_BOX_PYTHON_VENV=$ZEPHRY_BOX_PYTHON_VENV" >> ~/.bashrc
echo "export QEMU_EXTRA_FLAGS=\"-serial pty\"" >> ~/.bashrc

source ~/.bashrc
echo "ENVIRONMENT VARIABLES IN CONTAINER:"
printenv
