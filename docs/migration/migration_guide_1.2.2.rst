.. _migration_1.2.2:

Migration notes for |REPO_NAME| v1.2.2
#######################################

.. contents::
   :local:
   :depth: 3

This document describes the changes required or recommended when migrating deployed devices from |REPO_NAME| v1.2.0 or v1.2.1 to v1.2.2.
It also covers devices running the legacy |NCS| Matter door lock sample that you plan to migrate to the :ref:`doc_matter_door_lock_application` or :ref:`doc_aliro_matter_door_lock_application` on the nRF54L Series SoCs.

Required changes
****************

The following changes are mandatory to preserve access to the Matter Device Attestation Certificate (DAC) private key.

Matter DAC private key storage
==============================

.. note::
   Update the devices to the |REPO_NAME| v1.2.2 firmware using :ref:`matter_ota` or :ref:`dfu_ble_smp` before performing a factory reset.
   Updating the firmware after the factory reset does not restore a lost DAC private key.

#. Keep the ``CONFIG_CHIP_CRYPTO_PSA_DAC_PRIV_KEY_FALLBACK`` Kconfig option enabled (default) in the firmware you ship to devices that first booted |REPO_NAME| v1.2.0 or v1.2.1.
   Disabling it on these devices prevents them from booting until they are re-provisioned with new factory data.
#. Follow the migration path that matches the firmware that was first running on your devices, as described in the `ncs-matter Add-on v1.0.1 migration paths`_.
   In the |REPO_NAME|, the paths are as follows:

   * Path A — Devices first programmed with the in-tree |NCS| Matter firmware and later updated to |REPO_NAME| v1.2.0 or v1.2.1.
     These devices store the DAC private key in the Cryptographic Accelerator Engine (CRACEN) Key Management Unit (KMU).
     This includes devices first programmed with |REPO_NAME| v1.1.x or earlier, and devices first programmed with firmware based on the legacy |NCS| :doc:`Matter door lock <nrf:samples/matter/lock/README>` sample.
   * Path B — Devices first programmed with |REPO_NAME| v1.2.0 or v1.2.1.
   * Path C — New devices first programmed with |REPO_NAME| v1.2.2.

   Devices that store the DAC private key in KMU and are updated directly to |REPO_NAME| v1.2.2, without running v1.2.0 or v1.2.1 in between, do not require any changes.

Validating the migration
************************

Validate the updated firmware on a test device as described in the `ncs-matter Add-on v1.0.1 recommended validation`_ section, including Matter commissioning after the firmware update and factory reset.

For the complete description of the required and recommended changes, see the `ncs-matter Add-on v1.0.1 migration guide`_.
