.. _aliro_advanced_features:

Advanced Aliro protocol features
################################

.. contents::
   :local:
   :depth: 2

Beyond standard Aliro authentication, the CSA specification defines optional protocol phases for faster repeat unlocks and tiered authorization.
The |REPO_NAME| can enable these features independently through Kconfig.

This page describes the Expedited-fast and Step-up phases supported in the |REPO_NAME|, including authentication flow, build options, and provisioning requirements.
For the core Aliro stack and transport layers, see :ref:`aliro_integration`.

Expedited-fast phase
********************

The Expedited-fast phase speeds up authentication after a User Device has completed a successful Expedited-standard unlock at least once.
It is suited to high-traffic doors where repeat users should not wait through the full authentication exchange on every approach.

Overview
========

During the first successful authentication of a User Device (Expedited-standard phase), the Reader derives and stores a persistent symmetric key (Kpersistent) associated with that User Device's Access Credential.
On later attempts, the User Device can use Kpersistent to shorten the authentication exchange.

Authentication flow
===================

The Expedited-fast phase works as follows:

#. The User Device includes an encrypted cryptogram in its AUTH0 response, encrypted with its stored Kpersistent key.
#. The Reader tries to decrypt the cryptogram with each stored Kpersistent key until one matches.

   * If decryption succeeds, the Reader establishes a secure channel with the matched Kpersistent key, bypassing the full Expedited-standard flow.
   * If decryption fails for all stored keys, the Reader falls back to the Expedited-standard phase.

This reduces transaction latency for returning User Devices while keeping cryptographic validation.

Build and configuration
=======================

The following Kconfig options control Expedited-fast phase support and Kpersistent storage limits.
Set them in the application's :file:`prj.conf` or pass along with other build options to the ``west build`` command.
For example, to build the Aliro Access Control Application with Bluetooth LE and UWB transport and Expedited-fast phase support on the nRF5340 DK:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -Daliro-access-control-app_SNIPPET=uwb_qm35 -DCONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE=y


.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE``
     - Enables Expedited-fast phase support in the Aliro stack integration.
   * - ``CONFIG_MAX_NUMBER_OF_KPERSISTENT``
     - Maximum number of Kpersistent keys stored for Expedited-fast authentication.
       The default value is ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS``.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS``
     - Maximum number of Access Credential public keys stored in the :ref:`aliro_access_manager`.
       This value constrains the effective Kpersistent key limit.

.. note::
   The maximum number of Kpersistent keys must match the maximum number of stored Access Credential public keys in the :ref:`aliro_access_manager`.
   Mismatched limits are not supported and may cause undefined behavior.

Step-up phase
*************

The Step-up phase adds authorization based on Access Documents.
It is suited to sensitive areas where the Reader must verify user identity attributes and access rights beyond a standard credential check.

Overview
========

The Step-up phase extends authentication beyond Expedited-standard and Expedited-fast.
After a secure channel is established, the Reader can request an Access Document from the User Device.
The document carries signed authorization data, such as user identity attributes and access rights.

Authentication flow
===================

The Step-up phase works as follows:

#. After Expedited-standard authentication, the Reader may request an Access Document from the User Device.
#. The User Device presents its Access Document with cryptographically signed authorization data.
#. The Reader validates the document by verifying:

   * The digital signature
   * The access rights granted to the User Device

#. The :ref:`aliro_access_manager` applies the access policy based on the verification result.

Build, configuration, and provisioning
======================================

The following Kconfig options control Step-up phase support and credential storage limits.
Set them in the application's :file:`prj.conf` or pass along with other build options to the ``west build`` command.

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_STEP_UP_PHASE``
     - Enables Step-up phase support in the Aliro stack integration.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_ACCESS_CREDENTIAL_MAX_STORED_KEYS``
     - Maximum number of Access Credential public keys stored in the Access Manager during Step-up authorization.
   * - ``CONFIG_DOOR_LOCK_ACCESS_MANAGER_CREDENTIAL_ISSUER_MAX_STORED_KEYS``
     - Maximum number of Credential Issuer public keys stored for Access Document signature verification.
       See :file:`applications/*/src/aliro/access_manager/Kconfig` for defaults and help text.

.. note::
   In the Matter and Aliro Door Lock Application, ``CONFIG_DOOR_LOCK_STEP_UP_PHASE`` and ``CONFIG_DOOR_LOCK_EXPEDITED_FAST_PHASE`` are enabled by default.
   Credential Issuer keys are provisioned through Matter during normal setup.

When Matter support is disabled, provision Credential Issuer public keys manually before using Step-up.
From the serial console, run the following command:

.. code-block:: console

   uart:~$ dl provisioning CI_key set <key id> <65-byte public key in hex without 0x>

For full provisioning instructions, see :ref:`aliro_testing_provisioning_cli`.

For example, to build the Aliro Access Control Application with Step-up phase support on the nRF5340 DK:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -DCONFIG_DOOR_LOCK_STEP_UP_PHASE=y

Reader certificates
*******************

The Reader certificate lets the Reader authenticate itself to the User Device during the Expedited-standard phase.
It is suited to deployments where the User Device must confirm that it is communicating with a genuine Reader before completing authentication.

Overview
========

The Reader certificate is an optional X.509 certificate that binds the Reader's public key to a Reader System Issuer Certificate Authority (CA).
The Reader stores the certificate together with the Reader System Issuer CA public key in non-volatile storage.
When both are provisioned, the Reader presents the certificate to the User Device, which verifies its signature against the Issuer CA public key and confirms the Reader's identity.

Authentication flow
===================

The Reader certificate exchange works as follows:

#. After the ``AUTH0`` exchange, the Reader checks whether a certificate is provisioned.
#. If a certificate is present, the Reader sends it to the User Device with the ``LOAD_CERT`` command.
   Large certificates are transferred in multiple chunks using APDU chaining.

#. The User Device verifies the certificate signature against the Reader System Issuer CA public key.
#. After successful verification, the Reader continues with the Expedited-standard phase (``AUTH1``).

If no certificate is provisioned, the Reader skips the ``LOAD_CERT`` step and proceeds directly from the ``AUTH0`` response to the Expedited-standard phase.

Build and configuration
=======================

The following Kconfig options control Reader certificate support and certificate storage limits.
Set them in the application's :file:`prj.conf` or pass along with other build options to the ``west build`` command.

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Description
   * - ``CONFIG_DOOR_LOCK_READER_CERTIFICATE``
     - Enables Reader certificate support, including certificate storage and the ``reader`` shell commands.
       It is enabled by default in the Aliro Access Control Application and disabled by default in the Matter and Aliro Door Lock Application.
   * - ``CONFIG_DOOR_LOCK_ALIRO_READER_STORAGE_CERTIFICATE_MAX_SIZE``
     - Maximum size, in bytes, of the Reader certificate stored on the device.
       The default value is ``512``.

For example, to build the Aliro Access Control Application with Reader certificate support on the nRF5340 DK, run the following command:

.. code-block:: bash

   west build -p -b nrf5340dk/nrf5340/cpuapp applications/aliro-access-control-app -- -DCONFIG_DOOR_LOCK_READER_CERTIFICATE=y

For instructions on generating, provisioning, and testing Reader certificates, see :ref:`aliro_testing_reader_certificate`.
