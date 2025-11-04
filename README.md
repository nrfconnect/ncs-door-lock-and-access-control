# ncs-door-lock-app
NCS manifest repo for the door lock reference application.

## Getting started

Before getting started, set up the nRF Connect SDK development environment.

Follow the official [Installing the nRF Connect SDK](https://docs.nordicsemi.com/bundle/ncs-3.2.0-preview2/page/nrf/installation/install_ncs.html) guide and complete the following steps:

- [Update operating system](https://docs.nordicsemi.com/bundle/ncs-3.2.0-preview2/page/nrf/installation/install_ncs.html#update_operating_system)
- [Install prerequisites](https://docs.nordicsemi.com/bundle/ncs-3.2.0-preview2/page/nrf/installation/install_ncs.html#install_prerequisites)
- [Install the nRF Connect SDK toolchain](https://docs.nordicsemi.com/bundle/ncs-3.2.0-preview2/page/nrf/installation/install_ncs.html#install_the_nrf_connect_sdk_toolchain)

### Initialization

First, initialize the workspace folder (``door-lock-workspace``) where
the ``ncs-door-lock-app`` and all nRF Connect SDK modules will be cloned.

Run the following commands:

```shell
# launch nRF Connect toolchain for v3.2.0-preview2 release
nrfutil sdk-manager toolchain launch --ncs-version v3.2.0-preview2 --shell
```

```shell
# initialize workspace for the ncs-door-lock-app (v0.4.0 release)
west init -m https://github.com/nrfconnect/ncs-door-lock-app --mr v0.4.0 door-lock-workspace
```

```shell
# update nRF Connect SDK modules
cd door-lock-workspace
west update
```

For detailed instructions on building and testing the `ncs-door-lock-app` application, see the technical documentation in the `docs` directory.
To build this documentation locally, follow the steps in the section below.

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

The documentation will be generated in the ``build`` folder.
To view the built documentation, open ``docs/build/html/index.html`` in your web browser.
