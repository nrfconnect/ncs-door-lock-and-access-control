.. _testing_ble_nordic_uart:

Bluetooth LE Nordic UART Service (NUS)
######################################

.. contents::
   :local:
   :depth: 2

The Nordic UART Service (NUS) is a Bluetooth LE service that allows for remote control of the door lock using predefined commands.
This feature demonstrates the use of an out-of-band access control mechanism, unrelated to the Aliro and Matter protocols.

Enabling the NUS feature
************************

Enable the NUS feature depending on your configuration: 

.. tabs::

   .. group-tab:: With Matter disabled (Aliro only)

      Build the application with the ``bt_nus`` snippet.
      For example, if you are using the nRF5340 DK, run the following command:

      .. code-block:: console

         west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=bt_nus

   .. group-tab:: With Matter enabled (Aliro and Matter)

      Build the application with the ``matter`` snippet and enable the ``CONFIG_CHIP_NUS`` Kconfig option.
      For example, if you are using the nRF5340 DK, run the following command:

      .. code-block:: console

         west build -p -b nrf5340dk/nrf5340/cpuapp app -- -DSNIPPET=matter -DCONFIG_CHIP_NUS=y

Using the Bluetooth LE NUS service
**********************************

You can use the service in the following ways, depending on your configuration:

.. tabs::

   .. group-tab:: With Matter disabled (Aliro only)

      The application registers ``Lock`` and ``Unlock`` commands that can be sent from the `nRF Toolbox mobile application`_.
      You can register additional custom commands for the NUS service by using the ``RegisterCommand`` method from the ``NUSService`` class in application code.

      1. Pairing - Connect to the device advertised as `AliroDL` using passkey: ``123456``.
      2. Send Command - Send "Unlock" command using NUS RX characteristic.
      3. Result - You will see the following output in the device serial console:

         .. code-block:: console

            <inf> door_lock_app: Unlock command received

      Additionally, if you enable the ``CONFIG_DOOR_LOCK_LOCK_SIM_INDICATOR`` Kconfig option, the lock simulator indicator (green **LED 2**) lights up when the lock is unlocked.

   .. group-tab:: With Matter enabled (Aliro and Matter)

      Follow the detailed instructions on how to use NUS service with Matter provided in the `Matter door lock NUS`_ page.
