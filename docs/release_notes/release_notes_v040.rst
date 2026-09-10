.. _release_notes_v040:

Release notes for |REPO_NAME| v0.4.0
####################################

.. note::
  |EXPERIMENTAL_NOTE|

The following updates were introduced in this release.

* Added:

  * Matter support.
  * Integration of the Matter door lock cluster server in the |APP_NAME|.
  * Support for provisioning of Aliro door lock reader and user credentials using Matter.

* Updated:

  * nRF Connect SDK revision to v3.1.0.
  * STM RFAL drivers to the STSW-ST25RFAL005 revision.
  * QM35 libraries to latest internal Qorvo delivery.
  * Moved CLI and persistent storage implementation from Aliro stack to the application layer.

* Fixed:

  * An issue where the watchdog was expiring for an NFC session upon test execution (AL-335).
  * An issue where occasional timeout was occuring in the Reader when executing the RD-NFC-STDTXN-2.0 test from the test harness (AL-239).
  * An issue where an undefined access decision was occuring when executing the RD-NFC-STDTXN-2.0 test harness case in a loop (AL-282).
