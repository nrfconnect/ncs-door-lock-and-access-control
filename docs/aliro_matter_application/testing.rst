.. _testing:
.. _testing_environment:

Testing
#######

.. contents::
   :local:
   :depth: 2

The following pages will guide you through setting up the test environment, provisioning credentials, and verifying the |app_name| using the Aliro Test Harness and Matter-based tools.

To complete the testing process, you will need the following:

* The Nordic Semiconductor’s :ref:`development kit (DK) <hw_requirements>`, which serves as the Reader in the door lock component.
  It must be attached to an NFC card reader expansion board and optionally to a UWB module.
* The Aliro Test Harness, which acts as the User Device and simulates unlocking of the door lock.
  See the :ref:`hw_requirements` page for more details.
* `Matter controller tools`_  and `Matter over Thread tools`_ for device commissioning and testing the provisioning of Aliro credentials from the Matter controller.
  When testing with the Apple smart home ecosystem, you must have an iPhone running iOS version 26 or later and an Apple Matter controller, such as a HomePod mini.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   testing/setting_up_test_harness
   testing/cli_provisioning
   testing/testing_certificate_reader
   testing/provisioning_with_matter
   testing/verification_and_testing
   testing/cli_reference
   testing/nus
