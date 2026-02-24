.. _testing_cli_ref:

Command-line reference
######################

.. contents::
   :local:
   :depth: 2

This page lists commonly used command-line commands for inspecting device state, checking firmware versions, and managing stored keys on the door lock device.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Command
     - Description
   * - ``dl info``
     - Displays the Aliro library revision and the NFC reader in use.

       Example output::

          Aliro version: v0.2.0-22-g7da4b2e
          NFC reader: ST25R100

   * - ``dl btaddr``
     - Displays the Bluetooth LE address of the device.

   * - ``uwb qm35_fw_version``
     - Displays the firmware version of the QM35825 SoC.

       This command is available only when the application is built with the ``uwb_qm35`` snippet.
   * - ``dl kpersistent list``
     - Lists all Kpersistent keys currently stored on the device, including their index, ID, and public key.
   * - ``dl kpersistent clear <index>``
     - Clears a specific Kpersistent key by its index.

       Example::

          dl kpersistent clear 0

   * - ``dl provisioning``
     - Sets or gets provisioning credentials required by the Aliro protocol for authentication.

   * - ``dl reader``
     - Manages reader data (Identifier, private/public keys, certificate, Reader System Issuer CA public key, and Group Resolving Key).

   * - ``dl kpersistent clear all``
     - Clears all Kpersistent keys stored on the device.

   * - ``dl factory_reset``
     - Performs a factory reset on the device, clearing all stored data and settings.

.. note::
   The ``dl kpersistent`` commands are available only if the application is built with the
   ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` Kconfig option enabled.

Reader command details
**********************

The ``dl reader`` command provides the following subcommands:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Command
     - Description
   * - ``dl reader identifier [32-byte-hex]``
     - Gets or sets the full 32-byte Reader identifier.
   * - ``dl reader group_id [16-byte-hex]``
     - Gets or sets the Reader group identifier (first 16 bytes).
   * - ``dl reader group_sub_id [16-byte-hex]``
     - Gets or sets the Reader group sub-identifier (last 16 bytes).
   * - ``dl reader private_key set <32-byte-private-key-hex>``
     - Sets the Reader private key.
   * - ``dl reader private_key clear``
     - Clears the Reader private key.
   * - ``dl reader private_key list``
     - Prints the Reader private key status and its public key if set.
   * - ``dl reader certificate list|set|clear``
     - Lists, stores, or clears the Reader certificate (requires ``CONFIG_DOOR_LOCK_READER_CERTIFICATE``).
   * - ``dl reader issuer_public_key list|set|clear``
     - Lists, stores, or clears the Reader System Issuer CA public key (requires ``CONFIG_DOOR_LOCK_READER_CERTIFICATE``).
   * - ``dl reader group_resolving_key list|set|clear``
     - Lists, stores, or clears the Group Resolving Key (requires ``CONFIG_DOOR_LOCK_BLE_UWB``).
