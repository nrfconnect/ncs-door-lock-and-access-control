.. _testing_provisioning_cli:

Provisioning with CLI
#####################

.. contents::
   :local:
   :depth: 2

The following page describes the available methods for provisioning Aliro credentials on the door lock device using the command-line interface.
Once you have :ref:`Set up the Aliro Test Harness <setting_up_the_aliro_test_harness>`, complete the following steps:

#. Generate a Reader key pair:

   .. code-block:: console

      cd scripts
      python3 generate_keypair.py --verbose

   Save both values:

   * ``READER_PRIV`` - Reader private signing key (32 bytes, hex).
   * ``READER_PUB`` - Reader public key (65 bytes, hex).

#. Set the ``dut_reader_public_key`` field in the Test Harness project configuration to ``READER_PUB``.

#. Provision the Reader private signing key (``READER_PRIV``) in the DUT using the following ``dl reader`` shell command:

   .. code-block:: console

      uart:~$ dl reader private_key set <32-byte private key in hex without 0x>

   .. note::

      Once the required credentials are provisioned (Reader identifier and Reader private signing key), the device will automatically start the Aliro stack.

#. Install the Reader identifier in the Reader device.

   .. tabs::

      .. tab:: Full Reader identifier

         Install the complete 32-byte Reader identifier using a single command.

         .. code-block:: console

            dl reader identifier <32-byte_reader_identifier_in_hex_without_0x>

         Example:

         .. code-block:: console

            dl reader identifier 00113344667799AA00113344667799AA113344667799AA00113344667799AA00

      .. tab:: Group and sub-identifier

         Install the Reader group identifier and Reader group sub-identifier separately.
         Each value is 16 bytes.

         .. code-block:: console

            dl reader group_id <16-byte_reader_group_identifier_in_hex_without_0x>
            dl reader group_sub_id <16-byte_reader_group_sub_identifier_in_hex_without_0x>

         Example:

         .. code-block:: console

            dl reader group_id 00113344667799AA00113344667799AA
            dl reader group_sub_id 113344667799AA00113344667799AA00

         Running the same commands without specifying a value returns the currently stored identifier.

         Example:

         .. code-block:: console

            dl reader group_id

         Example output:

         .. code-block:: console

            00113344667799AA00113344667799AA

      .. tab:: Test harness configuration

         Instead of installing the identifier directly on the device, you can configure the Reader group identifier in the test harness project configuration.

         Set the value next to the ``dut_reader_group_identifier`` field.

#. Set the ``th_access_credential_public_key`` in the DUT using the following ``dl provisioning`` shell command:

   .. code-block:: console

      uart:~$ dl provisioning AC_key set <key id> <65-byte public key in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl provisioning AC_key set 0 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efa

   .. note::

      You can also use the following commands:

      * ``uart:~$ dl provisioning AC_key clear <key id>`` - This command clears the public key stored in the DUT by its ID (for example, uart:~$ dl provisioning AC_key clear 0).
      * ``uart:~$ dl provisioning AC_key clear all`` - This command clears all public keys stored in the DUT.
      * ``uart:~$ dl provisioning AC_key list`` - This command lists all public keys stored in the DUT.

      For example:

      .. code-block:: console

         uart:~$ dl provisioning AC_key list
         [0]: 04742df736d0fc9be978c45b00e8fdf7cea684ea105ae574c1505a2c24ab6198e3125b7f1b7e1d134c55ece69681ba8ecc18a3836dc5199c759f31e8ccf17e3efb
         [1]: (null)
         [2]: (null)
         [3]: (null)
         [4]: (null)
         [5]: (null)
         [6]: (null)
         [7]: (null)
         [8]: (null)
         [9]: (null)

#. Optionally, set the ``dut_credential_issuer_public_key`` in the DUT using the following ``dl provisioning`` shell command:

   .. code-block:: console

      uart:~$ dl provisioning CI_key set <key id> <65-byte public key in hex without 0x>


   For example:

   .. code-block:: console

      uart:~$ dl provisioning CI_key set 0 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137

   .. note::

      You can also use the following commands:

      * ``uart:~$ dl provisioning CI_key clear <key id>`` - This command clears the public key stored in the DUT by its ID (for example, ``uart:~$ dl provisioning CI_key clear 0``).
      * ``uart:~$ dl provisioning CI_key clear all`` - This command clears all public keys stored in the DUT.
      * ``uart:~$ dl provisioning CI_key list`` - This command lists all public keys stored in the DUT.

      For example:

      .. code-block:: console

         uart:~$ dl provisioning CI_key list
         [0]: 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137

#. Optionally, set the Credential Issuer Certificate Authority (CA) public key in the DUT using the following ``dl provisioning`` shell command:

   .. code-block:: console

      uart:~$ dl provisioning CI_CA_key set <65-byte public key in hex without 0x>

   For example:

   .. code-block:: console

      uart:~$ dl provisioning CI_CA_key set 047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137

   .. note::

      You can also use the following commands:

      * ``uart:~$ dl provisioning CI_CA_key get`` - This command retrieves the Credential Issuer CA public key stored in the DUT.
      * ``uart:~$ dl provisioning CI_CA_key clear`` - This command clears the Credential Issuer CA public key stored in the DUT.

      For example:

      .. code-block:: console

         uart:~$ dl provisioning CI_CA_key get
         047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137
