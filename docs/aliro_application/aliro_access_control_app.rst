.. _doc_aliro_access_control_application:

|ALIRO_APP_NAME|
################

The |ALIRO_APP_NAME| runs on the Nordic Semiconductor's :ref:`supported SoCs <hw_requirements>` and utilizes the Aliro stack for access protocol and communication with a User Device over Near Field Communication (NFC) or Bluetooth® LE (paired with ultra wideband).

Aliro standardizes the interaction that lets a phone or wearable act as a digital key at an opening, and does not dictate how the reader connects to the rest of the ecosystem.

In addition, the application supports optional Bluetooth® LE–based features, such as the Nordic UART Service (NUS) and Bluetooth® LE Secure Device Firmware Update (SMP DFU), depending on the configuration.
This application focuses exclusively on Aliro access control functionality without Matter integration, making it ideal for standalone Aliro implementations and commercial access control systems.

The following pages describe the application's available configuration options and building, running and testing instructions.


.. toctree::
   :maxdepth: 2
   :glob:
   :caption: Subpages:

   building_and_running.rst
   testing.rst
   firmware_update.rst
   troubleshooting.rst