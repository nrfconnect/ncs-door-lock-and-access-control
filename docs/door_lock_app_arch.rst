.. _addon_architecture:

|APP_NAME| architecture
#######################

The |APP_NAME| runs on the nRF54L15 SoC and utilizes the Aliro stack for access protocol and communication with user device over Near Field Communication (NFC) or Bluetooth® LE.
See the following diagram for an architecture overview:

.. _arch_overview:

.. figure:: /images/door_lock_app_arch.svg
   :scale: 100%
   :alt: nRF Door Lock Reference Application architecture.

   |APP_NAME| architecture overview.

The |APP_NAME| is build using the :ref:`nRF Connect SDK <sdk_set_up>`, which includes the Zephyr RTOS with all necessary modules.

The Aliro stack implements the Access Protocol logic, Aliro-specific cryptographic primitives, and communication with the user device.
The interfaces layer is a bridge connecting the Aliro stack to the Zephyr OS modules through specific backends that implement the following components required by the Aliro: crypto, NFC and, ultra wideband (UWB).
This layer additionally allows to utilize other, custom backends for the crypto, NFC and UWB components by implementing the provided API.
By default, the |APP_NAME| uses backends shown in the :ref:`architecture overview <arch_overview>`.
Both the Aliro stack and the interfaces layers are placed in the :file:`lib/aliro` directory.

The RF Abstraction Layer (NFC RFAL) handles communication with the STMicroelectronics :ref:`NFC integrated circuit (IC) <hw_requirements_nfc_reader>`.
This layer contains drivers, platform abstraction layer (PAL) and the NFC protocol stack, covering evrything from physical characteristic to the application layer.

Communication with external IC's is done through the Serial Peripheral Interface (SPI) bus.

The `Platform Security Architecture (PSA)`_ API provides a portable programming interface for cryptographic operations and key storage across a wide range of hardware.
It is designed to be user-friendly while still providing access to the low-level primitives essential for modern cryptography.

.. note::
   In the current implementation, the Bluetooth® LE transport protocol and UWB proximity sensing are not supported.
