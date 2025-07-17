.. _building_and_running:

Building and running
####################

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

   If you are using the nRF54L15 DK and `X-NUCLEO-NFC08A1`_, the command is:

      .. code-block:: bash

         west build -p -b nrf54l15dk/nrf54l15/cpuapp app -- -DCONFIG_ST25R3916B_DRV=y

   If you are using the nRF52840 DK and `X-NUCLEO-NFC05A1`_, the command is:

      .. code-block:: bash

         west build -p -b nrf52840dk/nrf52840 app -- -DCONFIG_ST25R3911_DRV=y

#. Once you have built the application, run the following command to flash it:

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
      [00:00:00.009,613] <inf> door_lock_app: Starting nRF Door Lock Reference Application for the nRF Connect SDK
