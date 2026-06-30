.. _testing_test_harness:
.. _setting_up_the_aliro_test_harness:
.. _testing_environment_configuration:

Setting up the Aliro Test Harness
#################################

.. contents::
   :local:
   :depth: 2

This page provides instructions on setting up the Test Harness and getting Access Credentials from it.
In case you do not have access to `Aliro Certification Tool`_ repository, see the :ref:`hw_requirements_test_harness` section for further guidance.

.. note::
   All examples related to the `Aliro Certification Tool`_ are based on the `aliro-sve-v1.0`_ tag.

#. Follow the `Test harness usage instructions`_ in the `Aliro Certification Tool`_ repository.

#. Open your Test Harness project's JSON configuration and locate the ``dut_reader_public_key``, ``th_access_credential_public_key``, ``dut_reader_group_identifier``, and ``dut_reader_group_sub_identifier`` fields.

  .. figure:: /images/th_config.png
     :scale: 70%
     :alt: Test harness project configuration.

     Test harness project configuration.

Certification test parameters
*****************************

For the Aliro certification test run, use the Test Harness parameters from :file:`applications/doorlock/docs/certification_assets/aliro_certification_test_parameters.json`.
These values are aligned with the DUT provisioning commands used by the CLI testing guide.

The most important DUT-facing values are:

* ``dut_reader_public_key`` - ``043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252a0a4f291192157a95cb6eb202759428c00cd834998c5d0eab192ee8873c5d34ee``
* ``dut_reader_group_identifier`` - ``00113344667799AA00113344667799AA``
* ``dut_reader_issuer_group_identifier`` - ``00113344667799AA00113344667799AB``
* ``dut_reader_group_sub_identifier`` - ``113344667799AA00113344667799AA00``
* ``dut_reader_group_resolving_key`` - ``00000000000000000000000000000000``
* ``dut_reader_issuer_public_key`` - ``043928f322019d4757893bde6a0fe5e13e3e537b9ca0f549c0bd2f40f79060252a0a4f291192157a95cb6eb202759428c00cd834998c5d0eab192ee8873c5d34ee``
* ``dut_credential_issuer_public_key`` - ``047BA31938492E3F5E97BC91806B5835B5D9E426609139006711E5FB7A670EE4E12FC9F25396C013CC20166029D761A105DEA5E071E84A9E499920524CE2301137``
* ``dut_access_element_id`` - ``floor1``
