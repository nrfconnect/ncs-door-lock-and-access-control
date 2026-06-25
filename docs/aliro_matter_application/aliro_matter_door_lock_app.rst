
.. _doc_aliro_matter_door_lock_application:

|MATTER_ALIRO_APP_NAME|
#######################

The |MATTER_ALIRO_APP_NAME| runs on Nordic Semiconductor :ref:`supported SoCs <hw_requirements>` and combines the `Matter`_ protocol stack with the Aliro stack for access protocol and communication with a User Device over Near Field Communication (NFC) or Bluetooth® LE (paired with ultra-wideband).
In addition, the application supports optional Bluetooth® LE–based features, such as the Nordic UART Service (NUS) and Device Firmware Update over Simple Management Protocol (DFU SMP), depending on the configuration.
The application integrates with Matter, enabling provisioning of Aliro-specific credentials through the smart home ecosystem.

.. toctree::
   :maxdepth: 2
   :caption: Subpages:

   aliro_matter_access_control_application
   testing
   firmware_update
   troubleshooting
