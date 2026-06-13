.. _aliro_building_and_running:

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
        - ``west build -p -b build_target applications/aliro-access-control-app``
        - Recommended for new door lock designs.
      * - Debug (default)
        - `X-NUCLEO-NFC09A1`_
        - ``west build -p -b build_target applications/aliro-access-control-app -- -DCONFIG_ST25R200_DRV=y``
        - Supported, but not recommended for new products.

   For example, if you are using the nRF5340 DK and `X-NUCLEO-NFC12A1`_, the command is:

      .. code-block:: bash

         west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app

   For the nRF54L15 DK and `X-NUCLEO-NFC09A1`_, run:

      .. code-block:: bash

         west build -p -b nrf54l15dk/nrf54l15/cpuapp applications/aliro-access-control-app -- -DCONFIG_ST25R200_DRV=y

#. To build the application with Bluetooth LE transport and UWB, run:

   .. code-block:: bash

      west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -DCONFIG_DOOR_LOCK_BLE_UWB=y

.. note::
   The above command builds the application with Bluetooth LE transport and UWB interface enabled but to operate properly it requires the implementation of the UWB interface to be provided by the application.
   The default implementation of the UWB interface is based on the Qorvo QM35825 UWB Aliro adapter and requires the ``nrfconnect-sdk-qorvo`` west add-on.
   Enable it with the ``uwb_qm35`` application snippet (see :ref:`aliro_building_with_uwb`).
   Note that the ``uwb_qm35`` snippet has the ``CONFIG_DOOR_LOCK_BLE_UWB`` Kconfig option enabled already.

#. Refer to the build variants for additional transports and protocols:

   .. list-table::
      :header-rows: 1
      :widths: 40 60

      * - Configuration
        - Build command
      * - QM35825 UWB module using the Qorvo Arduino Interface Board
        - ``west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=uwb_qm35``
      * - Aliro standalone (NFC only)
        - ``west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app``

   The ``uwb_qm35`` snippet configures the NFC and UWB modules to share the same SPI bus.

.. _aliro_building_and_running_config_options:

Configuration options
**********************

You can customize the application behavior using the following Kconfig options:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Configuration option
     - Description
   * - ``CONFIG_BT_MAX_CONN``
     - Sets the maximum number of simultaneous Bluetooth LE connections supported by the system.
       This value defines the upper bound for the number of concurrent Aliro Bluetooth LE sessions.
   * - ``CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS``
     - Limits the maximum number of concurrent Aliro Bluetooth LE sessions.
       You can use it to further restrict the number of active sessions without reducing the total number of Bluetooth LE connections supported by the system.

Building debug and release versions
************************************

In the release configuration, the application is built with the following characteristics:

* Aliro stack logs are disabled.
* RFAL NFC driver logs are disabled.
* Power-management options are enabled.
* Unused peripherals are disabled using the board-specific ``*_release.overlay``.
* The device resets automatically on a fatal error.

The ``dl`` shell remains enabled in release builds to allow provisioning the required keys (see :ref:`Provisioning with CLI <aliro_testing_provisioning_cli>`).

To build the application in release mode:

   .. code-block:: bash

      west build -p -b <build_target> applications/aliro-access-control-app -- -DFILE_SUFFIX=release

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

      .. tab:: Release configuration

         UART logs are disabled.
         Connect to the serial console and verify that the ``dl`` shell command is available and responds (see :ref:`Provisioning with CLI <aliro_testing_provisioning_cli>`).

.. _aliro_building_with_uwb:

Building with UWB support
*************************

UWB builds require the ``nrfconnect-sdk-qorvo`` west add-on, which provides the Qorvo UWB stack and QM35 host driver.

#. Add ``nrfconnect-sdk-qorvo`` to the west manifest group filter and update the workspace:

   .. code-block:: bash

      west config manifest.group-filter -- +nrfconnect-sdk-qorvo
      west update

#. Build the application with the ``uwb_qm35`` snippet.
   For instance, to build the application with UWB support for the nRF5340 DK, run the following command:

   .. code-block:: bash

      west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=uwb_qm35

   .. note::
      To get access to the ``nrfconnect-sdk-qorvo`` repository, contact your local Qorvo support team.
      If your access to ``nrfconnect-sdk-qorvo`` is revoked, updating the west workspace will fail unless you remove the repository from the west manifest group filter by running the following command:

      .. code-block:: bash

         west config manifest.group-filter -- -nrfconnect-sdk-qorvo

.. _aliro_flashing_qm35_using_nrf53_dk:

Flashing the QM35 firmware
**************************

For more details on QM35 firmware management, see :ref:`aliro_firmware_update`.
