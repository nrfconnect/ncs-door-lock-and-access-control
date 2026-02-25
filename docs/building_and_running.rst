.. _building_and_running:

Building and running
####################

.. contents::
   :local:
   :depth: 2

In the :file:`project-workspace`, the |REPO_NAME| is placed in the :file:`ncs-door-lock-and-access-control` directory.
To build and run the application on one of the :ref:`supported development kits (DKs) <hw_requirements>`, complete the following steps:

1. Connect the DK to your computer using the **DEBUGGER** port on the DK and set the **POWER** switch to **ON**.

#. In the :file:`project-workspace` directory, navigate to the :file:`ncs-door-lock-and-access-control` folder.

#. Depending on the :ref:`NFC reader expansion board <hw_requirements_nfc_reader>` connected to the development kit, build the application by running the corresponding command:

   You can find the ``build_target`` of your device in the :ref:`hw_requirements_development_kit` section.

   .. list-table::
      :header-rows: 1

      * - Build type
        - NFC reader expansion board
        - Build command
        - Description
      * - Debug (default)
        - `X-NUCLEO-NFC12A1`_
        - ``west build -p -b build_target app``
        - Recommended for new door lock designs.
      * - Debug (default)
        - `X-NUCLEO-NFC09A1`_
        - ``west build -p -b build_target app -- -DCONFIG_ST25R200_DRV=y``
        - Supported, but not recommended for new products.

   For example, if you are using the nRF5340 DK and `X-NUCLEO-NFC12A1`_, the command is:

      .. code-block:: bash

         west build -p -b nrf5340dk/nrf5340/cpuapp app

   For the nRF54L15 DK and `X-NUCLEO-NFC09A1`_, run:

      .. code-block:: bash

         west build -p -b nrf54l15dk/nrf54l15/cpuapp app -- -DCONFIG_ST25R200_DRV=y

#. To build the application with Bluetooth LE transport and UWB, run:

   .. code-block:: bash

      west build -p -b nrf5340dk/nrf5340/cpuapp app -- -DCONFIG_DOOR_LOCK_BLE_UWB=y

.. note::
   The above command builds the application with Bluetooth LE transport and UWB interface enabled but to operate properly it requires the implementation of the UWB interface to be provided by the application.
   The default implementation of the UWB interface is based on the Qorvo QM35825 UWB Aliro adapter and can we enabled by using the ``uwb_qm35`` application snippet.
   Note that the ``uwb_qm35`` snippet has the ``CONFIG_DOOR_LOCK_BLE_UWB`` Kconfig option enabled already.

#. Refer to the build variants for additional transports and protocols:

   .. list-table::
      :header-rows: 1
      :widths: 40 60

      * - Configuration
        - Build command
      * - QM35825 UWB module using the Qorvo Arduino Interface Board
        - ``west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=uwb_qm35``
      * - Matter over Thread
        - ``west build -p -b nrf5340dk/nrf5340/cpuapp app -- -DSNIPPET='matter'``
      * - QM35825 UWB module with Matter over Thread
        - ``west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=uwb_qm35 -DSNIPPET='matter'``

   The ``uwb_qm35`` snippet configures the NFC and UWB modules to share the same SPI bus.

Building debug and release versions
************************************

In the release configuration, the application is built with the following characteristics:

* Aliro stack logs are disabled.
* RFAL NFC driver logs are disabled.
* Power-management options are enabled.
* Unused peripherals are disabled using the board-specific ``*_release.overlay``.
* The device resets automatically on a fatal error.

Note that shell commands availability differs by variant:

* In the Matter variant, the UART shell is disabled by default in release builds.
* In the Aliro standalone variant, the ``dl`` shell remains enabled in release builds to allow provisioning the required keys (see :ref:`testing_provisioning_cli`).

To build the application in release mode:

   .. code-block:: bash

      west build -p -b <build_target> app -- -DFILE_SUFFIX=release

#. Once you have built the application, flash it:

   .. code-block:: bash

      west flash

#. Verify if the application runs correctly:

   .. tabs::

      .. tab:: Debug configuration

         Connect to the DK with a terminal emulator that supports VT100/ANSI escape characters.
         It is recommended to use the `Serial Terminal app`_.
         See the `Testing and optimization`_ page in the |NCS| documentation for the required settings.

         .. note::
            |app_hwfc_enabled|

         Press the **RESET** button on the DK in order to refresh the application.
         You should see the following logs:

         .. code-block:: console

            *** Booting My Application v0.1.0-f0e5cf444fb0 ***
            *** Using nRF Connect SDK v2.9.0-7787b2649840 ***
            *** Using Zephyr OS v3.7.99-1f8f3dc29142 ***
            Starting nRF Door Lock and Access Control Application

         * Additionally, depending on the activated options, you will see the following:

           * For QM35 UWB:

             .. code-block:: console

                uwb: Initializing UWB device...
                hsspi_helpers: Awake frame sending supported by FW
                hsspi_helpers: Awake frame not received
                uwb: UWB device initialized successfully.

          * For Matter:

            .. code-block:: console

               Init CHIP stack
               [DL]OpenThread started: OK
               ...
               [ZCL]Door Lock server initialized

      .. tab:: Release configuration

         UART logs are disabled.
         Complete the following steps to verify if the application runs correctly:

         * Aliro standalone - Connect to the serial console and verify that the ``dl`` shell command is available and responds (see :ref:`testing_provisioning_cli`).
         * Matter - Verify the LED and button behavior described in :ref:`matter_ui`.

Building QM35 host driver from source
*************************************

If you have an access to the Qorvo repository with UWB stack and QM35 driver source code, you can build the application with QM35 support compiled from sources.

#. Add the ``nrfconnect-sdk-qorvo`` to the west manifest group filter and update the workspace to fetch the ``nrfconnect-sdk-qorvo`` repository:

   .. code-block:: bash

      west config manifest.group-filter -- +nrfconnect-sdk-qorvo
      west update

#. Build the application with the QM35 host driver compiled from source using the ``uwb_qm35_src`` snippet.
   For instance, to build the application with Matter and UWB support for the nRF5340 DK, run the following command:

   .. code-block:: bash

      west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=uwb_qm35_src

   .. note::
      To get an access to the ``nrfconnect-sdk-qorvo`` repository with UWB stack and QM35 driver source code, contact your local Qorvo support team.
      In case your access to the ``nrfconnect-sdk-qorvo`` is revoked, updating the west workspace will fail unless you remove the repository from the west manifest group filter by running the following command:

      .. code-block:: bash

         west config manifest.group-filter -- -nrfconnect-sdk-qorvo

.. _flashing_qm35_using_nrf53_dk:

Flashing the QM35 firmware using nRF5340 DK
*******************************************

To flash the QM35 firmware using the nRF5340 DK, complete the following steps:

#. Plug in the QM35 module and connect nRF5340 DK.

#. Navigate to the :file:`nrfconnect-sdk-qorvo/flash_app` directory and build the application:

   .. code-block:: bash

      west build -b nrf5340dk/nrf5340/cpuapp -p

#. Flash the application:

   .. code-block:: bash

      west flash

#. You should see the following output on the serial terminal:

   .. code-block:: console

      *** Booting My Application v2.0.0-07f725e8810a ***
      *** Using nRF Connect SDK v3.2.0-rc2-45be7e87c461 ***
      *** Using Zephyr OS v4.2.99-7b2862b457c3 ***
      [00:00:00.259,094] <inf> main: Zephyr UWB Example Application 07f725e8810a
      [00:00:00.364,410] <inf> main: Device is ready
      [00:00:00.366,485] <inf> main: Firmware version: 0.6.0rc1_12208268663
      [00:00:00.366,516] <inf> main: Firmware flavor: silver_lychee
      [00:00:00.534,210] <inf> main: Flashing the device...
      [00:00:04.632,873] <inf> main: Flashing successful
      [00:00:04.632,873] <inf> main: Resetting the device...
      [00:00:04.758,819] <inf> main: Reset successful
      [00:00:04.760,833] <inf> main: Device is ready
      [00:00:04.763,000] <inf> main: Firmware version: 0.6.0rc1_12208268663
      [00:00:04.763,031] <inf> main: Firmware flavor: silver_lychee
