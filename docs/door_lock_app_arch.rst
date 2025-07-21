.. _addon_architecture:

|APP_NAME| architecture
#######################

.. contents::
   :local:
   :depth: 2

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

The UWB interface allows you to use an UWB hardware module with the Aliro stack and Access Manager.
This interface operates when the Bluetooth LE transport is enabled.
Using ranging measurements, the Access Manager can detect the distance between the User Device and the Reader and grant or deny access based on the access policy.


.. _Access_decision_indicator:

Access decision indicator
*************************

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

Aliro Access Manager
********************

The Access Manager interface (:file:`access_manager.h`) provides a unified API for handling access control logic in the |APP_NAME|.
It allows the application to use different access control strategies by providing the appropriate implementation.

Selecting implementation
========================

You can select an Access Manager implementation by enabling one of the following Kconfig options:

* ``CONFIG_ACCESS_MANAGER_IMPLEMENTATION_DEFAULT`` (:file:`access_manager_impl_default`) - This option is the default implementation, designed to cover typical access control scenarios.
  It makes access decisions based on the proximity of the Aliro User Device (as measured by UWB ranging) and the stored public keys.
  You can adjust this implementation to your needs through Kconfig options.
  This implementation is integrated with the Access Decision Indicator module, which provides visual feedback about the result of access decisions (for example, LED indicator).
  To see available Kconfig options, refer to the :ref:`addon_architecture_kconfig_default` subsection.

* ``CONFIG_ACCESS_MANAGER_IMPLEMENTATION_CUSTOM`` (:file:`access_manager_impl_custom`) - This option allows you to provide your own logic by implementing the interface defined in the :file:`access_manager.h` file.
  Use this if you need to integrate the |APP_NAME| with custom access policies.

.. _addon_architecture_kconfig_default:

Kconfig options for default implementation
------------------------------------------

You can configure the default implementation through Kconfig options located under the following path: :file:`app/src/access_manager_impl_default/Kconfig`:

* ``CONFIG_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM`` - This option specifies the maximum allowed distance (in centimeters) measured by UWB ranging required for granting access to the User Device.
  If the distance exceeds this value, access is denied.
  If the Aliro User Device is within this distance, access is granted.

* ``CONFIG_ACCESS_MANAGER_MAX_STORED_KEYS`` - This option sets the maximum number of public keys that can be stored in the Access Manager.
  It determines the size of the statically allocated memory for Access Manager cache.

  .. note::
     Currently only a single public key is supported.
     For this reason, you should set this value to ``1``.

* ``CONFIG_AUTOMATIC_RANGING_SESSION_INITIATION``- Enabling this option allows the Reader Device to automatically initiate the UWB ranging session in case the User Device fails to initiate it once the Access Protocol has been finalized.

* ``CONFIG_RANGING_START_TIMEOUT_MS``- This option specifies in milliseconds the time the Reader Device waits before initiating the ranging session.
  To start ranging immediately after the Access Protocol is finalized, set the value to ``0``.

Configuring Access Manager
==========================

To configure the Access Manager behavior, set options in your project’s :file:`prj.conf` file.
For example, to allow a maximum distance of 50 cm, add the following line to your prj.conf:

.. code-block:: kconfig

   CONFIG_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM=50

To use a custom implementation, provide your own implementation by modifying the files in the :file:`access_manager_impl_custom` file, and build the application with the ``CONFIG_ACCESS_MANAGER_IMPLEMENTATION_CUSTOM`` Kconfig option enabled.

For instance:

.. code-block::

   west build -b nrf5340dk/nrf5340/cpuapp app -- -DCONFIG_ACCESS_MANAGER_IMPLEMENTATION_CUSTOM=y
