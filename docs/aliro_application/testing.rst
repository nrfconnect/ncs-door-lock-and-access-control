.. _aliro_testing:
.. _aliro_testing_environment:

Testing
#######

.. contents::
   :local:
   :depth: 2

The following pages will guide you through setting up the test environment, provisioning credentials, and verifying the |ALIRO_APP_NAME| using the Aliro Test Harness.

Prerequisites
*************

The test environment consists of the following major components:

* The Nordic Semiconductor's :ref:`development kit (DK) <hw_requirements>`, which serves as the Reader in the door lock component.
  It must be attached to an NFC card reader expansion board and optionally to a UWB module.
* The Aliro Test Harness, which acts as the User Device and simulates unlocking of the door lock.

See the :ref:`hw_requirements` page for more details.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   testing/setting_up_test_harness
   testing/cli_provisioning
   testing/testing_certificate_reader
   testing/verification_and_testing
   testing/cli_reference
   testing/nus