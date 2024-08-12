# Zephyr Lab for Embedded Firmware at HKA

Welcome to the practical exercises to the course _Embedded Firmware for the Internet of Things_ at HKA.
During this lab we will explore the [Zephyr RTOS](https://docs.zephyrproject.org/latest/index.html) using the [Nordic Thingy91](https://docs.nordicsemi.com/bundle/ug_thingy91/page/UG/thingy91/intro/frontpage.html).

The Thingy91 is a two-processor system containing a few environmental sensors as well as an LTE/GPS chip and a separate Bluetooth chip.
We won't cover every aspect of that board, though.

# How to use this repo
We will use a containerized development alongside Visual Studio Code. Still, it is much easier if you work on a Linux machine.
Open this repo in VS Code and select `Reload in container`. This will open the repo inside a ready-to-use development container for Zephyr.

The tasks will gradually appear in the repo as the course progresses.

# How to build and flash
In order to build for the first time, execute the following command inside the `app` directory:
```bash
west build -p -b thingy91_nrf9160
```

If you have the NRF9160DK use the following command:
```bash
west build -p -b nrf9160dk_nrf9160
```


To rebuild the app, a simple `west build` or `west flash` is needed.

To flash the application onto the device use `west flash`.

## Note for Windows users
If you are flashing on Windows using `nrfjprog` use this command from a Windows terminal inside the app directory:

```bash
nrfjprog -f NRF91 --sectorerase --reset --verify --program .\\build\\zephyr\\zephyr.hex
```