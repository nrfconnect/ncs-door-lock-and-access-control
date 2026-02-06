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
