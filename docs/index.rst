.. _index:

|REPO_NAME|
########################

In combination with the |NCS|, the |REPO_NAME| provides a complete reference for building Aliro- and Matter-compatible locks and access control readers for both residential and commercial applications (see the `main repository <ncs-door-lock-and-access-control_>`_ for the Add-on's source code).
The reference integrates multiple wireless technologies, including Bluetooth Low Energy (Bluetooth LE), Ultra-Wideband (UWB), NFC, Thread, and Wi-Fi.
This allows you to :ref:`choose the appropriate technology for your specific use case <solution_overview>`.


The nRF Door Lock and Access Control Add-on is built on an open, vendor-agnostic architecture.
Platform abstraction APIs let you integrate a UWB SoC and an NFC transceiver from any silicon vendor. 
Within this open framework, the Add-on provides a validated turnkey path through native integration of the UWB SDK for Qorvo's QM35825 UWB SoC and the NFC driver for STMicroelectronics' ST25R300. For step-by-step instructions on connecting a third-party UWB module, see :ref:`Integrating a third-party UWB chip <uwb_custom_integration>`. 
For step-by-step instructions on connecting a third-party NFC transceiver, see :ref:`Integrating a third-party NFC chip <nfc_custom_integration>`.

The reference can support Aliro alone, Matter alone, or both protocols, depending on the specific use case and product requirements.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   solution_overview.rst
   hardware_requirements.rst
   software_requirements.rst
   wireless_technologies.rst
   applications.rst
   components.rst
   aliro_application_interactions.rst
   other_addons.rst
   release_notes.rst
   known_issues_and_limitations.rst
