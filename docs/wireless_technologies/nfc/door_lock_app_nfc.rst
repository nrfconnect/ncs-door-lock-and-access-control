.. _wireless_technologies_nfc:
.. _door_lock_app_nfc:

Near Field Communication (NFC)
##############################

.. contents::
   :local:
   :depth: 2

Near Field Communication (NFC) is a short-range wireless communication technology that operates at 13.56 MHz with a typical range of a couple of centimeters.
NFC enables data exchange between devices through electromagnetic induction, with devices operating in different roles: an NFC Reader (initiator) that generates the RF field and communicates with NFC tags (targets) that can be either passive (powered by the Reader's field) or active (self-powered).
This technology is ideal for contactless applications like payment systems, data sharing, and device pairing.

In access control applications, NFC provides an intuitive tap-to-unlock experience: the user holds a smartphone or wearable near the Reader antenna for instant authentication.
NFC is a mandatory transport in the Aliro protocol defined by the `Connectivity Standards Alliance`_ (CSA), where the Aliro Access Protocol authenticates credentials over the short-range NFC link.

For a general introduction to Aliro and how NFC fits alongside other transports, see :ref:`wireless_technologies_aliro` and the `Aliro Technology`_ page.

Overview
********

In the |REPO_NAME|, NFC is used exclusively as an Aliro transport layer.
The reader polls for a User Device in NFC-A listen mode, runs the Aliro Access Protocol and grants or denies access based on the authentication result.

NFC is well suited for deliberate, proximity-based access because of the following reasons:

* Reliable unlock when the user intentionally taps the device against the Reader.
* Close proximity between the User Device and the Reader limits remote threats during authentication.
* Low power consumption during idle polling compared with always-on ranging.
* Ease of use: only one NFC Aliro session can be active at a time.

Role in the Add-on
==================

Nordic nRF SoCs used in |REPO_NAME| support NFC tag mode natively, but Aliro requires NFC reader functionality on the Reader side.
To implement it, this reference allows connecting an external NFC reader IC over SPI and provides the integration layer and Aliro stack interfaces on the source code level to interact with it.

By default the |REPO_NAME| supports the STMicroelectronics' ST25R300 NFC reader and uses the `RFAL <https://www.st.com/resource/en/user_manual/um2890-rfnfc-abstraction-layer-stmicroelectronics.pdf>`_ (RF Abstraction Layer) driver.

Platform abstraction APIs allow replacing the default STMicroelectronics driver with a transceiver from another vendor.

To learn how the layering between application, Aliro stack, and NFC radio works, see :ref:`aliro_application_interactions`.

To see details about the NFC readers and expansion boards supported by default, see :ref:`hw_requirements_nfc_reader`.

Aliro protocol requirements
===========================

When NFC is used as the Aliro transport, the Reader and User Device follow the NFC-specific requirements of the Aliro Access Protocol:

* The reader operates in Poll Mode and supports NFC-A with T4T platform and ISO-DEP.
* The User Device operates in Listen Mode with the same technology support.
* Aliro Access Protocol commands are exchanged as APDUs after an application SELECT.

For protocol-level details, refer to the `Aliro protocol`_ documentation in the |NCS| and the Aliro specification published by the CSA.

.. toctree::
   :maxdepth: 2
   :glob:
   :caption: Subpages:

   nfc_integration.rst
   nfc_custom_integration.rst
   nfc_power_measurements.rst
