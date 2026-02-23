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

   * - ``dl install``
     - Sets or gets reader identifiers (group identifier and group sub-identifier).
   
   * - ``dl kpersistent clear all``
     - Clears all Kpersistent keys stored on the device.

   * - ``dl factory_reset``
     - Performs a factory reset on the device, clearing all stored data and settings.

.. note::
   The ``dl kpersistent`` commands are available only if the application is built with the
   ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` Kconfig option enabled.
