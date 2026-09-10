.. _release_notes_v031:

Release notes for |REPO_NAME| v0.3.1
####################################

.. note::
  |EXPERIMENTAL_NOTE|

The following updates were introduced in this release.

* Fixed:

  * An issue where ranging measurements with the QM35825 module were not delivered to the application.
    The QM35825 module's ranging measurements are now correctly delivered to the application.
    The distance between the Reader and User Device can now be displayed in the logs.
  * An issue with Access Manager not supporting multiple Aliro sessions.
    Access Manager now supports multiple Aliro and UWB sessions.
  * An issue where only one User Device could be provisioned to the Reader.
    The application now allows provisioning multiple User Devices to the Reader.

* Changed:

  * The Access Manager can now autonomously terminate the Aliro and UWB ranging sessions.
  * The ``dl provisioning AC_key`` shell command was extended to support multiple Access Credential public keys.
