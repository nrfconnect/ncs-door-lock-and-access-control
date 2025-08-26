# ncs-door-lock-app
NCS manifest repo for the door lock reference application.

## Getting started

Before getting started, you must set up the nRF Connect SDK development environment.

Follow the official [Getting Started Guide](https://docs.nordicsemi.com/bundle/ncs-3.1.0/page/zephyr/develop/getting_started/index.html) article: execute the following steps:

- [Select and Update OS](https://docs.nordicsemi.com/bundle/ncs-3.1.0/page/zephyr/develop/getting_started/index.html#select_and_update_os)
- [Install dependencies](https://docs.nordicsemi.com/bundle/ncs-3.1.0/page/zephyr/develop/getting_started/index.html#install_dependencies)
- [Get Zephyr and install Python dependencies](https://docs.nordicsemi.com/bundle/ncs-3.1.0/page/zephyr/develop/getting_started/index.html#get_zephyr_and_install_python_dependencies)
- [Install the Zephyr SDK](https://docs.nordicsemi.com/bundle/ncs-3.1.0/page/zephyr/develop/getting_started/index.html#install_the_zephyr_sdk)

### Initialization

You must first initialize the workspace folder (``door-lock-workspace``) where
the ``ncs-door-lock-app`` and all nRF Connect SDK modules will be cloned.
Run the following commands:

```shell
# initialize workspace for the ncs-door-lock-app (main branch)
west init -m https://github.com/nrfconnect/ncs-door-lock-app --mr main door-lock-workspace
```

```shell
# update nRF Connect SDK modules
cd door-lock-workspace
west update
```

Install additional Python requirements with ``pip``:

```shell
pip3 install -r zephyr/scripts/requirements.txt
pip3 install -r nrf/scripts/requirements.txt
```

Setting up the build environment:

```shell
# export a Zephyr CMake package
west zephyr-export
# define the required environment variable
source zephyr/zephyr-env.sh
```

### Building and running

To build the door lock application, run the following commands:

```shell
cd ncs-door-lock-app
west build -b nrf54l15dk/nrf54l15/cpuapp app
```

Once you have built the application, run the following command to flash it:

```shell
west flash
```

## Building local documentation

To build the documentation locally, you must complete a minimal setup.

Navigate to the ``docs`` folder, and install Sphinx and other documentation dependencies by running the following command:

```shell
pip install -r requirements.txt
```

Once successfully installed, you can build the Sphinx documentation from the ``docs`` folder:

```shell
make html
```

The output is stored in the ``build`` folder.
