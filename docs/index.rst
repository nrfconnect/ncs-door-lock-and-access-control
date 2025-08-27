.. _index:

|APP_NAME| for the |NCS|
########################

.. note::
  |EXPERIMENTAL_NOTE|

|APP_NAME| is a sample application that demonstrates how to integrate the Aliro and Matter stacks to implement fully functional door lock firmware.

Aliro is the new industry-standard access control and communication protocol, offering a secure, convenient, and consistent experience for users using smartphones, wearables, or other smart digital devices to unlock doors and openings.
Aliro features several key features that distinguish it from existing access protocols:

* Interoperability and compatibility - Ensures seamless interaction between access readers, such as electronic locks and access control readers, and User Devices like a smartphone and wearables.
  The standardized solution allows manufacturer-independent devices and readers to work together without compromising security.
* Flexibility - Does not dictate how your digital door lock or access control reader connects to the rest of your ecosystem.
* Protocol support - Supports various transport protocols, including mandatory Near Field Communication (NFC), Bluetooth® LE or Bluetooth LE with Ultra-Wideband (UWB).

`Matter <Matter marketing note_>`_ is an industry-standard smart home wireless application protocol, offering a secure and reliable communication between IoT accessories and smart home ecosystems.
Matter offers several key features:

* Interoperability - Ensures seamless interaction between IoT accessories and smart home ecosystems of different manufacturers.
* Flexibility - Depending on the use case, Matter can be implemented using different IPv6-based transport protocols: Wi-Fi, Thread, or Ethernet.
* Easy setup - Uses Bluetooth® LE for device discovery and provisioning.
* Security - Provides encrypted data exchange, confidentiality and integrity of the communication as well as authentication of the devices.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   door_lock_app_arch.rst
   other_addons.rst
   hardware_requirements.rst
   software_requirements.rst
   building_and_running.rst
   testing.rst
   troubleshooting.rst
   release_notes.rst
   known_issues.rst
