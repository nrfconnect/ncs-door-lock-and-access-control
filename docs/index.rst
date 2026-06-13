.. _index:

|REPO_NAME|
########################

In combination with the |NCS|, the |REPO_NAME| provides a complete reference for building Aliro- and Matter-compatible locks and access control readers for both residential and commercial applications (see the `main repository <ncs-door-lock-and-access-control_>`_ for the Add-on's source code).
The reference integrates multiple wireless technologies, including Bluetooth Low Energy (Bluetooth LE), Ultra-Wideband (UWB), NFC, Thread, and Wi-Fi.
This allows you to :ref:`choose the appropriate technology for your specific use case <solution_overview>`.

The |REPO_NAME| natively integrates the UWB SDK for Qorvo's QM35825 UWB SoC and the NFC driver for STMicroelectronics' ST25R300.
However, platform abstraction APIs allow for easy integration of a UWB SoC and an NFC transceiver from any silicon vendor.
For step-by-step instructions on connecting a third-party UWB module, see :ref:`uwb_custom_integration`.

The reference can support Aliro alone, Matter alone, or both protocols, depending on the specific use case and product requirements.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   solution_overview.rst
   wireless_technologies.rst
   aliro_application_interactions.rst
   other_addons.rst
   hardware_requirements.rst
   software_requirements.rst
   release_notes.rst
   known_issues_and_limitations.rst
   applications.rst
