.. _addon_architecture:

|APP_NAME| architecture and configuration
#########################################

.. contents::
   :local:
   :depth: 2

The |APP_NAME| runs on the Nordic Semiconductor's :ref:`supported SoCs <hw_requirements>` and utilizes the Aliro stack for access protocol and communication with a User Device over Near Field Communication (NFC) or Bluetooth® LE (paired with ultra wideband).

In addition, the application supports optional Bluetooth® LE–based features, such as the Nordic UART Service (NUS) and Bluetooth® LE Secure Device Firmware Update (SMP DFU), depending on the configuration.

You can also use the application with Matter for provisioning the Aliro-specific credentials by the smart home ecosystem.
The following page describes the application's architecture and its available configuration options.

Overview
********

The |APP_NAME| architecture can be represented as follows:

.. _arch_overview:

.. figure:: /images/door_lock_app_arch.svg
   :scale: 100%
   :alt: nRF Door Lock Reference Application architecture.

   |APP_NAME| architecture overview.

The application is built using the :ref:`nRF Connect SDK <sdk_set_up>`, which includes the Zephyr RTOS with all necessary modules.

The Aliro stack implements the Access Protocol logic, Aliro-specific cryptographic primitives, and communication with the User Device.
The interfaces layer is a bridge connecting the Aliro stack to the application through specific backends that implement the following components required by the Aliro:

* NFC
* Ultra wideband (UWB)
* Bluetooth LE
* Crypto

This layer additionally allows to utilize other, custom backends for the crypto, NFC and UWB components by implementing the provided API.
By default, the |APP_NAME| uses backends shown in the :ref:`architecture overview <arch_overview>`.
The Aliro stack library files are placed in the :file:`lib/aliro` directory.

The RF Abstraction Layer (NFC RFAL) handles communication with the STMicroelectronics :ref:`NFC integrated circuit (IC) <hw_requirements_nfc_reader>`.
This layer contains drivers, platform abstraction layer (PAL) and the NFC protocol stack, covering evrything from physical characteristic to the application layer.
You can choose three NFC sensitivity options, configurable through Kconfig (see :file:`drivers/nfc/stm/nfc_configs/Kconfig`):

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_RFAL_WAKE_UP_MODE_STRICT``
     - Offers lower sensitivity for increased robustness against noise.
   * - ``CONFIG_RFAL_WAKE_UP_MODE_RELAXED``
     - Provides higher sensitivity, allowing detection of weaker NFC signals, but may increase sensitivity to noise.
   * - ``CONFIG_RFAL_WAKE_UP_MODE_DEFAULT``
     - Uses the default RFAL configuration, suitable for general use cases where no specific tuning is required.

Communication with external IC's is done through the Serial Peripheral Interface (SPI) bus.

The `Platform Security Architecture (PSA)`_ API provides a portable programming interface for cryptographic operations and key storage across a wide range of hardware.
It is designed to be user-friendly while still providing access to the low-level primitives essential for modern cryptography.

The `Bluetooth LE stack <Bluetooth LE Controller_>`_ provides the necessary functionality for Bluetooth LE communication.

When the Aliro service is stopped and other Bluetooth LE services, such as Bluetooth NUS or SMP DFU, are active, the advertising payload is updated dynamically to reflect the active services.

.. _door_lock_app_arch_bluetooth_le:

Bluetooth LE transport
**********************

Bluetooth LE transport is supported on the `nRF5340 DK`_ platform and is configured to operate in the peripheral role, allowing the device to advertise its presence and accept connections from Bluetooth LE central devices.
The advertising payload is generated according to the Aliro specification, based on the ``reader group identifier`` and ``reader group sub-identifier``.
The payload updates automatically whenever these values change, ensuring that the device always advertises the latest group information.

.. note::
   The device is preconfigured with default values for the ``reader group identifier`` and ``reader group sub-identifier``.
   You can provide custom values using the ``dl install`` command through the console to override the defaults.
   For more information about setting these values, see the :ref:`testing_environment_configuration` section.

When a Bluetooth LE central device connects, the Aliro stack manages the Bluetooth LE session, handling connection events, security, and data exchange over a dedicated L2CAP channel with Aliro-specific GATT services.
This Bluetooth LE channel is a prerequisite for establishing the subsequent UWB session.
Each session uses unique keys, which are destroyed after termination.
In case of NFC transport, only one session can be active at a time.

Kconfig options
===============

When additional BLE services (such as NUS or DFU SMP) are enabled alongside Aliro BLE/UWB transport, ``CONFIG_BT_MAX_CONN`` must be increased to support concurrent connections.
The application automatically sets ``CONFIG_BT_MAX_CONN=2`` when ``CONFIG_DOOR_LOCK_BLE_UWB`` and either ``CONFIG_DOOR_LOCK_BLE_NUS`` or ``CONFIG_DOOR_LOCK_DFU_BLE_SMP`` are enabled.
You can override this default by explicitly setting ``CONFIG_BT_MAX_CONN`` to a different value in your project’s :file:`prj.conf` file.

You can configure Bluetooth LE transport parameters, such as buffer sizes, MTU, GATT database, L2CAP channels, TX power, and PHY through Kconfig (see :file:`lib/aliro/Kconfig.ble.defconfig`).

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_BT_MAX_CONN``
     - Sets the maximum number of simultaneous Bluetooth LE connections supported by the system.
       This value defines the upper bound for the number of concurrent Aliro Bluetooth LE sessions.
   * - ``CONFIG_DOOR_LOCK_BLE_UWB_MAX_SESSIONS``
     - Limits the maximum number of concurrent Aliro Bluetooth LE sessions.
       By default, this option follows the value of ``CONFIG_BT_MAX_CONN``.
       You can use it to further restrict the number of active sessions without reducing the total number of Bluetooth LE connections supported by the system.

       * Each session consumes resources, such as RAM and PSA key slots, therefore the maximum number of the active sessions should be determined empirically based on the requirements of the final application.

.. _uwb_interface:

UWB interface
*************

The UWB interface allows you to use an UWB hardware module with the Aliro stack and Access Manager.
This interface operates when the Bluetooth LE transport is :ref:`enabled <door_lock_app_arch_bluetooth_le>`.
Using ranging measurements, the Access Manager can detect the distance between the User Device and the Reader and grant or deny access based on the access policy.

Qorvo QM35 interface implementation
===================================

The Qorvo QM35 implementation is the default implementation for the UWB interface.
It is based on the Qorvo QM35825 UWB module and you can enable it through the ``uwb_qm35`` snippet.

You can also provide custom implementation using different UWB hardware module.

Kconfig options
---------------

You can configure the Qorvo QM35 interface implementation using the following Kconfig options located under the :file:`app/src/platform/uwb_impl/uwb_qm35_impl/Kconfig` path:

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_UWB_MIN_RAN_MULTIPLIER``
     - Specifies the minimum RAN multiplier for UWB ranging blocks.
       This option controls the frequency of UWB ranging measurements received by the Reader.
       The supported time range is from 96 milliseconds, when the RAN multiplier is 1, to 24480 milliseconds, when the RAN multiplier is 255.
       The default value is ``2``, which corresponds to a measurement frequency of approximately 5 Hz.
   * - ``CONFIG_DOOR_LOCK_UWB_MAC_MODE_OFFSET``
     - Sets the offset between the two ranging blocks in MAC mode.
       The supported range is from 0 to 63, with a default value of ``0``.
   * - ``CONFIG_DOOR_LOCK_UWB_MAC_MODE_RANGING_ROUNDS``
     - Specifies the number of ranging rounds used in a ranging block.

       Available values include:

       * ``0`` – one ranging round (default)
       * ``1`` – two ranging rounds

   * - ``CONFIG_DOOR_LOCK_UWB_SESSION_LOGGING``
     - Enables logging of UWB session states, including status codes.
       This option is intended for debugging and monitoring UWB session behavior.

The number of simultaneous UWB ranging sessions is currently not limited by the application.
Each Aliro Bluetooth LE session can have its own UWB ranging session.
You can establish two simultaneous UWB ranging sessions.
Support for more than two simultaneous UWB ranging sessions has not been verified.

.. _Access_decision_indicator:

Access decision indicator
*************************

When access is granted and door lock is unlocked, a generic indicator is activated for a predefined period to visually confirm successful authentication.
By default, this indicator is a dedicated LED, but it can be adapted to other types of indicators, such as a buzzer.

You can set the duration of the indication in the :file:`app/src/platform/access_decision_indicator/Kconfig` file.
Use the ``CONFIG_ACCESS_DECISION_INDICATOR_STATE_DELAY_MS`` Kconfig option to specify the time in milliseconds.

You can select the hardware resource used for this indication by going to the device tree source file and setting the ``access-decision-indicator`` alias in the corresponding node.

.. code-block:: dts

  /{
    aliases {
      access-decision-indicator = &led2; // green LED2
    };
  };

Aliro Access Manager
********************

The Access Manager interface (:file:`access_manager.h`) provides a unified API for handling access control logic in the |APP_NAME|.
The default implementation (:file:`access_manager`) is designed to cover typical access control scenarios.
It makes access decisions based on the proximity of the Aliro User Device, as measured by UWB ranging, and the stored public keys.
It integrates with the Access Decision Indicator module, which provides visual feedback about access decisions, for example using LED indicator.

.. _addon_architecture_kconfig_default:

Kconfig options
===============

You can configure the Access Manager through Kconfig options located under the following path: :file:`app/src/aliro/access_manager/Kconfig`:

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM``
     - Specifies the maximum allowed distance, in centimeters, measured by UWB ranging for granting access to the User Device.
       If the measured distance exceeds this value, access is denied.
       If the Aliro User Device is within this distance, access is granted.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_EXIT_MARGIN_CM``
     - * Specifies an additional margin, in centimeters, added to ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM`` to determine when the door should be locked, referred to as the exit range.
         This margin prevents rapid open and close toggling when the measured distance fluctuates around the maximum allowed distance threshold.

         * Unlock behavior applies when the distance is less than or equal to ``MAX_ALLOWED_DISTANCE_CM``.
         * Lock behavior applies when the distance exceeds ``MAX_ALLOWED_DISTANCE_CM + EXIT_MARGIN_CM``.
         * The exit margin applies per UWB ranging session based on the previous session state.

       Set this option to ``0`` to disable the exit margin, causing the door to lock immediately when the distance exceeds ``MAX_ALLOWED_DISTANCE_CM``.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS``
     - Sets the maximum number of public keys that can be stored in the Access Manager.
       This option determines the size of the statically allocated memory for the Access Manager cache.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION``
     - Selects the method used by the Aliro Reader to terminate the UWB ranging session.
       This mechanism is important when the User Device does not terminate the ranging session on its own.

       Available suboptions include:

       * ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_DISABLED``
         Prevents the Access Manager from automatically terminating UWB ranging sessions.
         This is the default behavior.
         Sessions terminate only when explicitly requested by the User Device or when the BLE connection is lost.
         This option is useful when the User Device controls the session lifecycle.

       * ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_ACCESS_GRANTED``
         Terminates the UWB ranging session immediately after access is granted for the session.

       * ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_TERMINATE_SESSION_ON_TIMEOUT``
         Terminates the UWB ranging session after a specified timeout.
         The timeout, in milliseconds, is defined by the
         ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_SESSION_TIMEOUT_MS`` Kconfig option.
         By default, the timeout is set to ``10000`` milliseconds.

Configuring Access Manager
==========================

To configure the Access Manager behavior, set options in your project’s :file:`prj.conf` file.
For example, to allow a maximum distance of 50 cm, add the following line:

.. code-block:: kconfig

   CONFIG_DOOR_LOCK_ACCESS_MANAGER_MAX_ALLOWED_DISTANCE_CM=50

Matter support
**************

.. note::
  Currently, the |APP_NAME| supports Matter only with Thread technology.

The |APP_NAME| integrates with the `Matter`_ protocol stack provided by the |NCS| to enable seamless provisioning of Aliro-specific credentials through smart home ecosystems.

The |APP_NAME| implements smart home access control using the Matter door lock cluster, which is natively supported in the |NCS| by the `Matter door lock`_ sample.
|APP_NAME| uses most of the functionality of the Matter door lock sample.

For detailed information about Matter support in the |NCS|, see the following pages:

* `Matter overview`_ - Provides an introduction to Matter, covering its architecture, firmware upgrade process, and details on network commissioning and security.
* `Matter getting started`_ - Guides on developing Matter applications in the |NCS|, including the necessary tools and how to set up the testing environment.
* `Matter end product`_ - Discusses developing a Matter end product, detailing supported security features, recommended configurations, and the Matter certification process.

Aliro Expedited-fast phase support
**********************************

The |APP_NAME| optionally supports the Aliro Expedited-fast phase as defined in the Aliro specification.
The Expedited-fast phase provides an optimized authentication mechanism that enables faster subsequent transactions after an initial successful authentication of the User Device.

Overview
========

During the first successful authentication of a User Device (using the Expedited-standard phase), the Reader derives and stores a persistent symmetric key (Kpersistent) associated with that User Device's Access Credential.
In subsequent transaction attempts, the User Device can leverage this Kpersistent key to streamline the authentication process.

Authentication flow
===================

The Expedited-fast phase authentication flow operates as follows:

#. The User Device includes an encrypted cryptogram in its AUTH0 response, encrypted using its stored Kpersistent key.
#. The Reader attempts to decrypt the cryptogram using each stored Kpersistent key until it finds a match.

  * If decryption succeeds, the Reader establishes a secure channel using the matched Kpersistent key, bypassing the full Expedited-standard authentication flow.
  * If decryption fails for all stored Kpersistent keys, the Reader falls back to the Expedited-standard phase.

This optimization may reduce transaction latency for authenticated User Devices while maintaining security through cryptographic validation.

Enabling the feature
====================

To enable the Expedited-fast phase support, build the application with the ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` Kconfig option enabled.

For example, to build the application with Bluetooth LE and UWB transport support for nRF5340 platform, along with the Expedited-fast phase capability, run the following command:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp app -- -Dapp_SNIPPET=uwb_qm35 -DCONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE=y

Configuration
=============

You can apply the following configuration options:

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_MAX_NUMBER_OF_KPERSISTENT``
     - Sets the maximum number of persistent keys that can be stored by the system.
       This option defines the upper limit for the number of keys allocated in persistent storage.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS``
     - Defines the maximum number of Access Credential public keys that can be stored in the Access Manager.
       This value constrains the effective maximum number of persistent keys used for access credentials.

.. note::
   The maximum number of Kpersistent keys must match the maximum number of stored Access Credential public keys.
   Configurations with mismatched limits are not supported and may result in undefined behavior.

Aliro Step-up phase support
****************************

The |APP_NAME| optionally supports the Aliro Step-up phase as defined in the Aliro specification.
The Step-up phase provides an enhanced authentication mechanism that enables User Devices to present and verify Access Documents for authorization.

Overview
========

The Step-up phase extends the authentication capabilities beyond the Expedited-standard and Expedited-fast phases.
In this phase, the Reader can request and verify an Access Document received from the User Device.
Each Access Document contains additional authorization information such as the user identity attributes and access rights.

Authentication flow
===================

The Step-up phase authentication flow operates as follows:

#. After establishing a secure channel through Expedited-standard phase, the Reader may request an Access Document from the User Device.
#. The User Device presents its Access Document, which contains cryptographically signed authorization data.
#. The Reader validates the Access Document by verifying:

   * The digital signature of the document
   * The specific access rights granted to the User Device

#. Based on the verification results, the Reader makes an access decision according to the defined access policy.

Enabling the feature
====================

To enable the Step-up phase support, build the application with the ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` Kconfig option enabled.

For example, to build the application with Step-up phase support for nRF5340 platform, run the following command:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp app -- -DCONFIG_DOOR_LOCK_STEP_UP_PHASE=y

Configuration
=============

The Step-up phase uses the Access Manager interface to make authorization decisions based on the verified Access Document.

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS``
     - Sets the maximum number of Access Credential public keys that can be stored in the Access Manager.
       This limit applies during the Step-up phase, where authorization decisions are made based on the verified Access Document.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS``
     - Configures the maximum number of Credential Issuer public keys that can be stored.
       These keys are required during the Step-up phase to verify the digital signature of Access Documents.
       The keys must be provisioned into the Reader before the Step-up phase can be used.
       See :file:`app/src/aliro/access_manager/Kconfig` for details.

.. note::
   Manual provisioning of Credential Issuer public keys is required only when Matter support is disabled.
   When Matter support is enabled, the credentials are provisioned through the Matter protocol.

If Matter support is disabled, you can provision these keys using the following command through the serial console:

.. code-block:: console

   uart:~$ dl provisioning CI_key set <key id> <65-byte public key in hex without 0x>

For more information about provisioning Credential Issuer keys, see the :ref:`testing` page.
