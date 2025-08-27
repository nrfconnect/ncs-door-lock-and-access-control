.. _building_and_running:

Building and running
####################

.. contents::
   :local:
   :depth: 2

In the :file:`door-lock-workspace`, the |APP_NAME| is placed in the :file:`ncs-door-lock-app` directory.
To build and run the application on one of the :ref:`supported development kits (DKs) <hw_requirements>`, complete the following steps:

1. Connect the DK to your computer using the **DEBUGGER** port on the DK.
   Set the **POWER** switch to **ON**.

#. In the :file:`door-lock-workspace` directory, navigate to the :file:`ncs-door-lock-app` folder.

#. Depending on the :ref:`NFC reader expansion board <hw_requirements_nfc_reader>` connected to the development kit,
   build the application by running the corresponding command:

   +-----------------------+-----------------------+------------------------------------------------------------------------------------------------+
   | Build type            | X-NUCLEO-NFC board    | Command                                                                                        |
   +=======================+=======================+================================================================================================+
   | Debug (default)       | `X-NUCLEO-NFC09A1`_   | ``west build -p -b build_target app``                                                          |
   |                       +-----------------------+------------------------------------------------------------------------------------------------+
   |                       | `X-NUCLEO-NFC08A1`_   | ``west build -p -b build_target app -- -DCONFIG_ST25R3916B_DRV=y``                             |
   |                       +-----------------------+------------------------------------------------------------------------------------------------+
   |                       | `X-NUCLEO-NFC05A1`_   | ``west build -p -b build_target app -- -DCONFIG_ST25R3911_DRV=y``                              |
   +-----------------------+-----------------------+------------------------------------------------------------------------------------------------+

   You can find the ``build_target`` of your device in the :ref:`hw_requirements_development_kit` section.

   For example, if you are using the nRF5340 DK and `X-NUCLEO-NFC09A1`_, the command is:

      .. code-block:: bash

         west build -p -b nrf5340dk/nrf5340/cpuapp app

   For the nRF54L15 DK and `X-NUCLEO-NFC08A1`_, run:

      .. code-block:: bash

         west build -p -b nrf54l15dk/nrf54l15/cpuapp app -- -DCONFIG_ST25R3916B_DRV=y

#. You can also apply optional configurations depending on the modules used:

   * If you are using the `QM35825`_ UWB module with the Qorvo Arduino Interface Board, execute the following command to build the application specifically for this setup:

     .. code-block:: bash

      west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=uwb_qm35

     .. note::
      The snippet's configuration disables the NFC reader to enable the use of the UWB module with the Qorvo Arduino Interface Board.

   * For Matter over Thread, execute the following command to build the application:

     .. code-block:: bash

        west build -p -b nrf5340dk/nrf5340/cpuapp app -- -DSNIPPET='matter'

   * To build the application with `QM35825`_ UWB module support and Matter over Thread enabled, run:

     .. code-block:: bash

        west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=uwb_qm35 -DSNIPPET='matter'

#. Once you have built the application, flash it:

   .. code-block:: bash

      west flash

#. To verify if the application runs, connect to the DK with a terminal emulator that supports VT100/ANSI escape characters.
   It is recommended to use the `Serial Terminal app`_.
   See the `Testing and optimization`_ page in the |NCS| documentation for the required settings.

   .. note::
      |app_hwfc_enabled|

#. Press the **RESET** button on the DK in order to refresh the application.
   You should see the following logs:

   .. code-block:: console

      *** Booting My Application v0.1.0-f0e5cf444fb0 ***
      *** Using nRF Connect SDK v2.9.0-7787b2649840 ***
      *** Using Zephyr OS v3.7.99-1f8f3dc29142 ***
      door_lock_app: Starting nRF Door Lock Reference Application for the nRF Connect SDK

   Optionally, if you activated QM35 UWB support, you should also see the following logs:

   .. code-block:: console

      uwb: Initializing UWB device...
      hsspi_helpers: Awake frame sending supported by FW
      hsspi_helpers: Awake frame not received
      uwb: UWB device initialized successfully.

   Additionally, if you enabled Matter, you should also see the logs:

   .. code-block:: console

      Init CHIP stack
      [DL]OpenThread started: OK
      ...
      [ZCL]Door Lock server initialized
