
.. _doc_aliro_matter_door_lock_application:

|MATTER_ALIRO_APP_NAME|
#######################

The |MATTER_ALIRO_APP_NAME| runs on the Nordic Semiconductor's :ref:`supported SoCs <hw_requirements>` and combines the `Matter`_ protocol stack with the Aliro stack for access protocol and communication with a User Device over Near Field Communication (NFC) or Bluetooth® LE (paired with ultra wideband).
In addition, the application supports optional Bluetooth® LE–based features, such as the Nordic UART Service (NUS) and Bluetooth® LE Secure Device Firmware Update (SMP DFU), depending on the configuration.
The application integrates with Matter, enabling provisioning of Aliro-specific credentials through the smart home ecosystem.
The following pages describe the application's available configuration options and building, running and testing instructions.

.. toctree::
   :maxdepth: 2
   :glob:
   :caption: Subpages:

   building_and_running.rst
   testing.rst
   firmware_update.rst
   troubleshooting.rst
