# Zephyr Lab for Embedded Firmware at HKA

Welcome to the practical exercises to the course _Embedded Firmware for the Internet of Things_ at HKA.
During this lab we will explore the [Zephyr RTOS](https://docs.zephyrproject.org/latest/index.html) using the [Olimex LoRa devkit](https://www.olimex.com/Products/IoT/LoRa/LoRa-STM32WL-DevKit/open-source-hardware).

The devkit features an STM32WLE5 SoC which integrates an ARM Cortex-M4 and a LoRa radio module.
Additionally, the devkit contains several sensors like the AHT20 for temperature and humidity measurements, as well as a 3-axis magnetometer, a light-sensor, a user button and LEDs.
During this course you will learn how to use Zephyr RTOS to read the sensors, process their measurements and send them to a remote location using LoRa.

# How to use this repo
The development environment will be provided to you in a virtual machine that can be run using [VirtualBox](https://www.virtualbox.org/).
Make sure to also install the [VirtualBox guest additions](https://docs.oracle.com/en/virtualization/virtualbox/6.0/user/guestadd-install.html) for USB tunneling support.

For those who cannot or do not want to use VirtualBox, follow the [toolchain setup instructions](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) on the Zephyr website for your OS.
Furthermore, in order to get `pyocd` support for flashing the board, run the following two commands after installing the toolchain:

```bash
pyocd pack --update
pyocd pack --install stm32wl
```

# How to build and flash

In the virtual machine all Zephyr projects are located under `~/zephyrproject/<project-name>`.
When you want to work on a project like this repo (or your final project), change to the project directory and run the following commands:

```bash
west config manifest.path <your-project-folder>
west update
```

This configures the west path (Zephyr's build tool) to use your `west.yml` as the manifest and pulls all the right dependency versions.

In order to build for the first time, execute the following command inside the `app` directory:
```bash
west build -p -b olimex_lora_stm32wl_devkit@D
```

To rebuild the app, a simple `west build` or `west flash` is needed.

To flash the application onto the device use `west flash`.

To start a GDB session run `west debug`.

You can see the log output of your application using the following command:
```bash
minicom -con -b 115200 -D /dev/ttyACM0
```

To exit minicom again, press `<Ctrl-a><Ctrl-x>` and then hit `<Enter>`.

> **Hint:** For debugging enable the config options `CONFIG_NO_OPTIMIZATIONS=y` and `CONFIG_DEBUG=y` if there is enough flash.
