.. _wireless_technologies_uwb:

Ultra-wideband (UWB)
####################

.. contents::
   :local:
   :depth: 2

Ultra-wideband (UWB) is a short-range radio technology that transmits over a very wide frequency spectrum (more than 500 MHz of bandwidth) using very short pulses at low energy levels.
These short pulses let a receiver measure the time of flight of a signal with high precision, which makes UWB well suited for accurate and secure distance measurement (ranging) between two devices.

In access control applications, UWB enables a hands-free, proximity-based unlock: the reader measures how far away the smartphone is and opens the lock automatically as the user approaches, then relocks when they move away.
In the Aliro protocol defined by the `Connectivity Standards Alliance`_ (CSA), UWB is an optional extension to the Bluetooth® LE transport that provides the secure ranging used to make these distance-based access decisions.

For a general introduction to Aliro and how UWB fits alongside other transports, see :ref:`wireless_technologies_aliro` and the `Aliro Technology`_ page.

.. note::

   This reference does not support the Aliro Bluetooth LE-only transport variant.

Overview
********

In the |REPO_NAME|, UWB is never used on its own.
Bluetooth LE carries the Aliro Access Protocol and the UWB ranging session setup, while UWB performs the secure ranging that measures the distance between the User Device and the reader.
After the User Device is authenticated over Bluetooth LE, the reader and the User Device establish a UWB ranging session, and the Aliro Access Manager uses the continuous distance reports to drive the lock.

UWB is well suited for hands-free, proximity-based access for the following reasons:

* Walk-up-and-unlock experience without the user having to tap the device against the reader.
* Secure distance measurement that is resistant to relay attacks.
* Configurable unlock and relock distances for stable hands-free behavior.
* Support for multiple concurrent ranging sessions.

Role in the add-on
==================

UWB is not implemented on the nRF SoC itself.
The |REPO_NAME| drives an external UWB module connected over SPI and provides the integration layer and Aliro stack interfaces on the source code level to interact with it.

As an example, the |REPO_NAME| supports the Qorvo `QM35825`_ UWB SoC through the ``nrfconnect-sdk-qorvo`` west add-on, which provides an adapter library that maps the Aliro UWB session semantics onto the Qorvo firmware.

Platform abstraction APIs allow replacing the default Qorvo module with a UWB radio from another vendor.

To learn how the layering between the application, the Aliro stack, and the UWB radio works, see :ref:`uwb_integration` and :ref:`aliro_application_interactions`.

To see details about the UWB module and expansion board supported by default, see :ref:`hw_requirements_uwb_module`.

Access policy
=============

The Aliro Access Manager translates UWB distance measurements into lock actions:

* **Unlock** — when the measured distance is at or below the configured maximum allowed distance.
* **Lock** — when the measured distance exceeds the allowed distance plus a configurable exit margin.

The exit margin prevents rapid lock and unlock toggling when the measured distance fluctuates around the threshold.

Front/back disambiguation
=========================

When front/back disambiguation is enabled, an additional UWB radar session determines whether the user is in front of or behind the door.
This prevents an unlock when the user is on the wrong side of the entry point.
See :ref:`uwb_disambiguation` for algorithm details and configuration parameters.

.. toctree::
   :maxdepth: 2
   :glob:
   :caption: Subpages:

   uwb_integration.rst
   uwb_custom_integration.rst
   uwb_disambiguation.rst
