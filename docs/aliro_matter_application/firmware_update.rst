.. _firmware_update:

Firmware update
###############

.. contents::
   :local:
   :depth: 2

This page will guide you through the available firmware update mechanisms for the |app_name|.

When built with QM35 Ultra-Wideband (UWB) support, the update image can also bundle QM35825 module firmware (:ref:`matter_qm35_firmware_upgrade`).
In-field QM35 image delivery is supported over Matter OTA and Bluetooth LE SMP DFU.
The application stores the image in external flash and applies it to the module during initialization.

For build variants and flashing instructions, see :ref:`aliro_matter_access_control_application`.

.. _matter_ota:

Matter OTA
**********

The application allows you to update the firmware of the device using the Matter Over-The-Air (OTA) update mechanism.
This allows you to remotely upgrade the door lock firmware without physical access to the device.

This guide uses CHIP Tool as a Matter controller and assumes you have a commissioned Matter device.
For details on CHIP Tool usage, see the `Matter chip-tool guide`_.

Prerequisites
=============

Before starting the OTA update process, ensure that:

* The Matter device is commissioned to the network (see the commissioning instructions in the :ref:`testing_verification` section).
* You have built the new Matter application with a higher version number than the one that is currently running on the device to be updated. The version is configured in the :file:`applications/matter-aliro-door-lock-app/VERSION` file.
  The :file:`matter.ota` file generated in the build directory will be used as the OTA firmware image file.
* You have the Matter OTA provider application available on your PC.
  You can download the precompiled OTA provider application from the `Matter fork release`_ page.

OTA update process
==================

The following steps will guide you through the complete OTA update process:

#. Start and run the OTA provider application to make the new firmware image accessible to the target device:

   .. code-block:: console

      chip-ota-provider-app -f <ota-file-path>

   Where ``<ota-file-path>`` is a path to the OTA firmware image file.
   For example:

   .. code-block:: console

      chip-ota-provider-app -f ./build/matter.ota

#. Commission and pair the OTA provider with the Matter network using the default pairing credentials:

   .. code-block:: console

      chip-tool pairing onnetwork <provider-node-id> <pin-code>

   Where:

     * ``<provider-node-id>`` is the node ID for the OTA provider (must be unique on the network)
     * ``<pin-code>`` is the setup PIN code for the OTA provider

   For example:

   .. code-block:: console

      chip-tool pairing onnetwork 2 20202021

#. Configure the Access Control List (ACL) to allow the target device access to the OTA provider:

   .. code-block:: console

      chip-tool accesscontrol write acl <acl-entries> <provider-node-id> <provider-endpoint-id>

   Where:

     * ``<acl-entries>`` refers to the Access Control List entries
     * ``<provider-node-id>`` is the node ID of the OTA provider
     * ``<provider-endpoint-id>`` is the endpoint ID of the ACL cluster

   For example:

   .. code-block:: console

      chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 2 0

   .. note::
      The ACL contains two entries: the first one grants administrative privileges to the controller (subject 112233), and the second one grants operational privileges to all subjects (subjects: null) for general OTA operations.

#. Register the target device to use the OTA provider.

   .. code-block:: console

      chip-tool otasoftwareupdaterequestor write default-otaproviders <attribute-value> <target-node-id> <target-endpoint-id>

   Where:

     * ``<attribute-value>`` is the attribute value
     * ``<target-node-id>`` is the node ID of the target door lock device
     * ``<target-endpoint-id>`` is the endpoint ID of the OTA Software Update Requestor cluster

   For example:

   .. code-block:: console

      chip-tool otasoftwareupdaterequestor write default-otaproviders '[{"fabricIndex": 1, "providerNodeID": 2, "endpoint": 0}]' 1 0

#. Initiate the OTA update by announcing the OTA provider to the target device to start the update process.

   .. code-block:: console

      chip-tool otasoftwareupdaterequestor announce-otaprovider <provider-node-id> <vendor-id> <announcement-reason> <provider-endpoint-id> <target-node-id> <target-endpoint-id>

   Where

     * ``<provider-node-id>`` is the node ID of the OTA provider
     * ``<vendor-id>`` is the vendor ID
     * ``<announcement-reason>`` is the announcement reason (``0`` = simple announcement)
     * ``<provider-endpoint-id>`` is the endpoint ID of the OTA Software Update Provider cluster
     * ``<target-node-id>`` is the node ID of the target device
     * ``<target-endpoint-id>`` is the endpoint ID of the OTA Software Update Requestor cluster

   For example:

   .. code-block:: console

      chip-tool otasoftwareupdaterequestor announce-otaprovider 2 0 0 0 1 0

#. Monitor the automatic OTA update process.
   Track the progress through the device and OTA provider logs.
   The device will reboot with the new firmware upon successful update:

   .. code-block:: console

      *** Booting My Application <new-version>-<sha> ***
      ...
      <inf> chip: [SWU] New firmware image confirmed

   .. note::
      The update process can take several minutes depending on the firmware size and network conditions.
      Do not power off the device during the update process.


.. _dfu_ble_smp:

DFU over Bluetooth LE SMP
*************************

The |MATTER_ALIRO_APP_NAME| supports field firmware updates over Bluetooth LE using the Simple Management Protocol (`SMP protocol`_).

See :ref:`door_lock_app_ble_smp` and :ref:`door_lock_dfu_smp_service`.

Prerequisites
=============

.. include:: /include/firmware_update_dfu_smp_prerequisites.txt

Enabling Bluetooth LE SMP advertising
=====================================

SMP advertising is not started automatically after boot, press **Button 3** to toggle it.

.. include:: /include/firmware_update_dfu_smp_shell_advertising.txt

Updating firmware
=================

Set the application version in the :file:`applications/matter-aliro-door-lock-app/VERSION` file before building the update image.

.. include:: /include/firmware_update_dfu_smp_updating.txt

.. _matter_qm35_firmware_upgrade:

QM35825 firmware upgrade
*************************

The application can update QM35825 UWB module firmware in addition to the main application image.
The QM35 image is stored in external flash as an extra MCUboot image managed by MCUboot and the partition manager.
At startup, the application compares the stored image version with the version running on the module and applies an update when the configured version policy matches (higher or different).
If QM35 initialization fails, the application retries the firmware upgrade regardless of the current version.
The maximum supported firmware size is 512 kB.

.. _flashing_qm35_using_nrf53_dk:

Initial QM35 module programming
===============================

Before first use of UWB on QM35 hardware, the module coprocessor must run firmware compatible with the host driver in your build.

You can program the module in either of the following ways:

Bundled with the application image
----------------------------------

Build with the ``uwb_qm35_dfu`` snippet together with ``uwb_qm35``, then flash the full image set with ``west flash --erase``.
See :ref:`matter_qm35_firmware_update_enable` below.

Qorvo flash_app on nRF5340 DK
-----------------------------

To program the module directly before building the door lock application, use the Qorvo ``flash_app`` tool in the `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository (see :ref:`matter_aliro_building_and_running_qm35_src`):

#. Connect the QM35825 module to the nRF5340 DK.
#. Navigate to :file:`qm35-aliro-sdk/flash_app` and build:

   .. code-block:: bash

      west build -b nrf5340dk/nrf5340/cpuapp -p

#. Flash the programmer firmware:

   .. code-block:: bash

      west flash

#. Confirm on the serial console that flashing completed successfully and the reported firmware version matches your target.

In-field QM35 update
====================

In-field QM35 image delivery is supported over Matter OTA and Bluetooth LE SMP DFU.
The application applies the bundled QM35 image during initialization when a newer or different version is detected.

Update flow
-----------

At startup the application:

#. Compares the running QM35 firmware version with the version stored in external flash.
#. Starts an update when the configured version policy matches (higher or different).
#. Resets the QM35 into firmware update mode and transfers the firmware from external flash to the module.
#. Resets the QM35 and re-initializes the UWB stack.
#. Verifies the installation after transfer completes.

.. note::
   During the update, the QM35 is temporarily unavailable.

.. _matter_qm35_firmware_update_enable:

Enabling QM35 firmware update
-----------------------------

Build with the ``uwb_qm35_dfu`` snippet together with ``uwb_qm35``.

.. note::
   The ``uwb_qm35_dfu`` snippet requires the ``uwb_qm35`` snippet to be enabled.
   For Bluetooth LE SMP DFU support, the ``dfu_smp`` snippet must also be enabled.

.. code-block:: bash

   west build -b nrf5340dk/nrf5340/cpuapp applications/matter-aliro-door-lock-app -- \
     -Dmatter-aliro-door-lock-app_SNIPPET='uwb_qm35;uwb_qm35_dfu;dfu_smp'

The QM35 firmware image is managed automatically during application initialization.
Flash the full image set with ``west flash --erase`` (see :ref:`aliro_matter_access_control_application`).

QM35 DFU configuration options
------------------------------

Control QM35 firmware update behavior using the following Kconfig options:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Configuration option
     - Description
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU``
     - Enables QM35 firmware upgrade support.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_HIGHER``
     - Updates only if the new version is higher.
       This is the default behavior.
   * - ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_DIFFERENT``
     - Updates if the version differs.

.. note::
   Use the ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_DIFFERENT`` option if you need to update from an older Qorvo firmware that reports a higher version number than the new firmware.
   For example, when downgrading from ``13.0.1rc2_10977260000`` to ``0.6.0rc1_12208268663``, the default ``CONFIG_DOOR_LOCK_ALIRO_UWB_QM35_DFU_VERSION_COMPARISON_HIGHER`` option would block the update, since the target version is lower.

By default, the firmware binary is taken from ``${ZEPHYR_QM35_ALIRO_SDK_MODULE_DIR}/firmware/qm35825.bin``.
To use a different binary, specify its absolute path with the ``QM35_IMAGE_PATH`` build argument:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/matter-aliro-door-lock-app -- -DQM35_IMAGE_PATH='/path/to/qm35825.bin'

Flashing
--------

When flashing the application, run the following command:

.. code-block:: console

   west flash --erase

The QM35 firmware is automatically programmed to external flash together with the main application.

.. note::
   For the nRF54LM20 DK, external flash support in ``west flash`` is not in a production state yet.
   Therefore, using ``nrfutil`` is currently required to program the firmware:

   .. code-block:: console

      nrfutil device --x-ext-mem-config-file applications/matter-aliro-door-lock-app/boards/nrf54lm20dk_spi_nrfutil_config.json program --firmware build/merged.hex --options verify=VERIFY_READ,ext_mem_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,reset=RESET_SOFT

QM35 firmware update monitoring
-------------------------------

Monitor QM35 firmware updates through logs:

.. code-block:: console

   [00:00:02.123,456] <inf> uwb: QM35 FW version comparison: stored=1.2.3 vs running=1.2.2
   [00:00:02.234,567] <inf> uwb: QM35 firmware update started
   [00:00:05.345,678] <inf> uwb: QM35 firmware chunk 1/10 transferred
   [00:00:10.456,789] <inf> uwb: QM35 firmware update completed successfully
   [00:00:11.567,890] <inf> uwb: QM35 FW revision: 1.2.3rc1_[...]

Check the current QM35 firmware version with the shell command:

.. code-block:: console

   uart:~$ uwb qm35_fw_version
