.. _firmware_update:

Firmware update
###############

.. contents::
   :local:
   :depth: 2

This page will guide you through the available firmware update mechanisms for the |app_name|.

.. _matter_ota:

Matter OTA
**********

If you build an application with Matter support (see :ref:`building_and_running` and the ``-DSNIPPET='matter'`` option), you can update the firmware of the device using the Matter Over-The-Air (OTA) update mechanism.
This allows you to remotely upgrade the door lock firmware without physical access to the device.

.. note::
   This guide uses CHIP Tool as a Matter controller and assumes you have a commissioned Matter device.
   For details on CHIP Tool usage, see the `Matter chip-tool guide`_.

.. note::
   Currently, Matter OTA does not support updating the firmware of the QM35 UWB module.

Prerequisites
=============

Before starting the OTA update process, ensure that:

* The Matter device is commissioned to the network (see the commissioning instructions in the :ref:`testing_verification` section).
* You have built the new Matter application with a higher version number than the one that is currently running on the device to be updated. The version is configured in the :file:`app/VERSION` file.
  The :file:`matter.ota` file generated in the build directory will be used as the OTA firmware image file.
* You have the Matter OTA provider application available on your PC.

.. note::
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

The DFU over Bluetooth LE Simple Management Protocol (SMP) allows for seamless firmware updates on devices using the SMP protocol over a Bluetooth LE connection.
By building the application with the ``-Dapp_SNIPPET=dfu_smp`` option, you can enable this feature and update your device's firmware wirelessly.

.. note::
   For applications without Matter support, DFU over Bluetooth LE SMP is supported only on the nRF5340 DK.
   To build Matter applications with DFU over Bluetooth LE SMP, add the ``-DCONFIG_CHIP_DFU_OVER_BT_SMP=y`` option to the build command.

Enabling Bluetooth LE SMP advertising
=====================================

When the application is built with the ``-DCONFIG_DOOR_LOCK_BLE_UWB=y`` option, the Bluetooth LE SMP advertising is enabled by default.

For the NFC-only applications, you can enable the Bluetooth LE SMP advertising by pressing a **Button 1** on the device to start or stop SMP advertising.

Alternatively, if Matter is not enabled, you can run the following shell command:

   .. code-block:: console

      dl dfu_smp_adv on

   To stop advertising, run:

   .. code-block:: console

      dl dfu_smp_adv off

Updating firmware
=================

Before initiating the OTA update process, ensure that the new application is built with a higher version number than the currently running version.
You can configure it in the :file:`app/VERSION` file.
There following methods are recommended for performing a firmware update over Bluetooth LE:

*  Using the `nRF Device Manager`_ - This application provides a user-friendly interface for performing DFU updates.
   See the Nordic Developer Academy course on `DFU over Bluetooth LE using device manager app`_.

*  Using the `Newt Manager`_ - This method allows you to perform DFU updates directly from the terminal.
   For details, see `DFU over Bluetooth LE using newt manager tool`_.
