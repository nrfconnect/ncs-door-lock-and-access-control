.. _addon_architecture:

|APP_NAME| architecture
#######################

The |APP_NAME| runs on the Nordic Semiconductor's :ref:`supported SoCs <hw_requirements>` and utilizes the Aliro stack for access protocol and communication with user device over Near Field Communication (NFC) or Bluetooth® LE.
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
The Aliro stack library files are placed in the :file:`lib/aliro` directory.

The RF Abstraction Layer (NFC RFAL) handles communication with the STMicroelectronics :ref:`NFC integrated circuit (IC) <hw_requirements_nfc_reader>`.
This layer contains drivers, platform abstraction layer (PAL) and the NFC protocol stack, covering evrything from physical characteristic to the application layer.

Communication with external IC's is done through the Serial Peripheral Interface (SPI) bus.

The `Platform Security Architecture (PSA)`_ API provides a portable programming interface for cryptographic operations and key storage across a wide range of hardware.
It is designed to be user-friendly while still providing access to the low-level primitives essential for modern cryptography.

The `Bluetooth LE Controller`_ provides the necessary functionality for Bluetooth LE communication.
Bluetooth LE transport is supported on the `nRF5340 DK`_ platform and is configured to operate in the peripheral role, allowing the device to advertise its presence and accept connections from Bluetooth LE central devices.
The advertising payload is generated according to the Aliro specification, based on the ``reader group identifier`` and ``reader group sub-identifier``.
The payload updates automatically whenever these values change, ensuring that the device always advertises the latest group information.

.. note::
   The device is preconfigured with default values for the ``reader group identifier`` and ``reader group sub-identifier``.
   You can provide custom values using the ``dl install`` command through the console to override the defaults.
   For more information about setting these values, see the :ref:`testing_environment_configuration` section.

When a Bluetooth LE central device connects, the Aliro stack manages the Bluetooth LE session, handling connection events, security, and data exchange over a dedicated L2CAP channel with Aliro-specific GATT services.
Each session uses unique keys, which are destroyed after termination.
In case of NFC transport, only one session can be active at a time.
For Bluetooth LE transport, the maximum number of concurrent sessions is limited by the ``ALIRO_BLE_TP_MAX_SESSIONS`` Kconfig option and defaults to the value of ``BT_MAX_CONN``.

To configure number of Bluetooth LE sessions, set the ``BT_MAX_CONN`` to maximum number of connections and optionally use the ``ALIRO_BLE_TP_MAX_SESSIONS`` to limit the number of sessions.

.. note::
   Each session consumes resources, such as RAM and PSA key slots, therefore the maximum number of the active sessions should be determined empirically based on the requirements of the final application.

You can configure Bluetooth LE transport parameters, such as buffer sizes, MTU, GATT database, L2CAP channels, TX power, and PHY through Kconfig (see :file:`lib/aliro/Kconfig.ble.defconfig`).

.. note::
   In the current implementation, the UWB proximity sensing is not supported.

.. _Access_decision_indicator:

Access decision indicator
=========================

When access is granted, a generic indicator is activated for a predefined period to visually confirm successful authentication.
By default, this indicator is a dedicated LED, but it can be adapted to other types of indicators such as a buzzer.

You can set the duration of the indication in the :file:`app/src/platform/access_decision_indicator/Kconfig` file.
Use the ``RESET_ACCESS_DECISION_INDICATOR_STATE_DELAY_MS`` Kconfig option to specify the time in milliseconds.

You can select the hardware resource used for this indication by going to the device tree source file and setting the ``access_decision_indicator`` property in the corresponding node.

.. code-block:: dts

   access_decision_indicator: access_decision_indicator {
       status = "okay";
       compatible = "access-decision-indicator";
       gpios = <&gpio2 10 GPIO_ACTIVE_HIGH>;
   };
