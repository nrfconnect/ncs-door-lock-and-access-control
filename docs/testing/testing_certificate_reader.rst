.. _testing_reader_certificate:

Testing Aliro Reader Certificate
################################

.. contents::
   :local:
   :depth: 2

The Aliro Reader Certificate is an optional X.509 certificate that allows a Reader device to authenticate itself to the User Device during the expedited standard phase of the Aliro protocol.
When a certificate is provisioned, the Reader sends a ``LOAD_CERT`` command after the ``AUTH0`` exchange, allowing the User Device to verify the Reader's identity.

This feature is controlled by the ``CONFIG_DOOR_LOCK_READER_CERTIFICATE`` Kconfig option (enabled by default for non-Matter builds).

Certificate generation and provisioning
***************************************

This section describes how to generate Reader certificates and provision them on the device under test (DUT).

Generating certificates
=======================

#. Generate an Issuer key pair for signing certificates:

   .. code-block:: console

      cd scripts
      python3 generate_keypair.py --verbose

#. Save the output:

   * ``ISSUER_PRIV`` - Issuer Private Key (32 bytes hex)
   * ``ISSUER_PUB`` - Issuer Public Key (65 bytes hex)

#. Generate a Reader certificate.
   You can choose between two certificate sizes:

   * Short certificate (~152 bytes) - It does not require APDU chaining.
     You can generate it by running the following command:

      .. code-block:: console

         python3 generate_reader_cert.py \
         --subject-pubkey <DUT_READER_PUBLIC_KEY> \
         --issuer-privkey <ISSUER_PRIV> \
         --output hex | xargs python3 compress_reader_cert.py
 

   * Long certificate (~270 bytes) - It requires APDU chaining (for testing chaining mechanism).
     You can generate it by running the following command:

      .. code-block:: console

         ./generate_max_size_cert.sh <DUT_READER_PUBLIC_KEY> <ISSUER_PRIV>

#. Save the compressed certificate HEX output for provisioning.

Configuring Test Harness
========================

Update your Test Harness project configuration with the following fields:

* ``dut_reader_issuer_public_key`` - Set to the ``ISSUER_PUB``.
* ``dut_reader_issuer_group_identifier`` - Set it to a value different from ``dut_reader_group_identifier``.
  Otherwise, you will not be able to test certificate-based authentication.

   .. code-block:: json

      {
      "config": {
         "test_parameters": {
            "dut_reader_public_key": "<DUT_READER_PUBLIC_KEY>",
            "dut_reader_group_identifier": "00113344667799AA00113344667799AB",
            "dut_reader_issuer_group_identifier": "FFEEDDCCBBAA998877665544332211FF",
            "dut_reader_issuer_public_key": "<ISSUER_PUB>"
         }
      }
      }

For the certification test parameters in :file:`applications/doorlock/docs/certification_assets/aliro_certification_test_parameters.json`, use:

.. code-block:: json

   {
      "test_parameters": {
         "dut_reader_group_identifier": "00113344667799AA00113344667799AA",
         "dut_reader_issuer_group_identifier": "00113344667799AA00113344667799AB",
         "dut_reader_issuer_public_key": "043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252a0a4f291192157a95cb6eb202759428c00cd834998c5d0eab192ee8873c5d34ee"
      }
   }

Provisioning DUT
================

Connect to the DUT through serial console and provision both the certificate and issuer public key:

#. Install the proper Reader group identifier (``dut_reader_issuer_group_identifier`` value):

   .. code-block:: console

      uart:~$ dl reader group_id <dut_reader_issuer_group_identifier>

#. Provision the Issuer Public Key:

   .. code-block:: console

      uart:~$ dl reader issuer_public_key set <ISSUER_PUB>

#. Provision the Reader Certificate (compressed):

   .. code-block:: console

      uart:~$ dl reader certificate set <COMPRESSED_CERT>

#. Verify provisioning:

   .. code-block:: console

      uart:~$ dl reader issuer_public_key list
      Issuer public key (65 bytes): <ISSUER_PUB>

      uart:~$ dl reader certificate list
      Reader certificate (XXX bytes): <COMPRESSED_CERT>

For the certification test parameters, provision the short Reader certificate with:

.. code-block:: console

   uart:~$ dl reader group_id 00113344667799AA00113344667799AB
   uart:~$ dl reader issuer_public_key set 043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252a0a4f291192157a95cb6eb202759428c00cd834998c5d0eab192ee8873c5d34ee
   uart:~$ dl reader certificate set 3081950402000030818e854200043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252a0a4f291192157a95cb6eb202759428c00cd834998c5d0eab192ee8873c5d34ee8648003045022100f509f4e64b31b5c8d4152158065b4eedd31c66d6e7b1f87975f837f5a3fe1235022063ee11a312731c4673382c7fcde101440767ff56654bf64595be802ec0ace3e1

For ``NFC_RDR_STANDARD_CERT_IN_LOAD_CERT_WITH_CHAINING``, replace the certificate value with the longer ``th_reader_certificate_chaining`` value from the certification test parameters JSON file.

.. note::
   The certificate size is limited by ``CONFIG_DOOR_LOCK_READER_CERTIFICATE_MAX_SIZE``. 
   By default it is set to 512 bytes.
   If you need to provision larger certificates, increase this value in your project configuration.

Running tests with certificates
*******************************

Once the certificate is provisioned, execute Test Harness test cases that include the ``LOAD_CERT`` command:

* ``NFC_RDR_STANDARD_CERT_IN_LOAD_CERT`` - Standard certificate test
* ``NFC_RDR_STANDARD_CERT_IN_LOAD_CERT_WITH_CHAINING`` - Certificate with APDU chaining

You can expect the following behavior:

* DUT sends ``LOAD_CERT`` command with the provisioned certificate.
* If you are using a long certificate, APDU chaining will be used (in multiple chunks).
* Test Harness verifies the certificate signature using ``ISSUER_PUB``.
* Transaction proceeds normally after validating the certificate.

Clearing certificates
=====================

To run standard tests without certificates, clear the provisioned data:

.. code-block:: console

   uart:~$ dl reader certificate clear
   uart:~$ dl reader issuer_public_key clear

After clearing, the DUT will skip the ``LOAD_CERT`` state and proceed directly from ``AUTH0`` response to ``AUTH1`` (expedited standard phase).
