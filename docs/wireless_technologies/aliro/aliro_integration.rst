.. _aliro_integration:

Aliro integration in the reference applications
###############################################

.. contents::
   :local:
   :depth: 2

This page explains how the Aliro stack is integrated into the |REPO_NAME|.
It covers the layered architecture, the application interface contract, and where the stack library lives in the source tree.

Architecture overview
*********************

.. figure:: /images/door_lock_app_arch_aliro.svg
   :scale: 100%
   :alt: General architecture of the application using the Aliro stack.

The Nordic Aliro solution is built on the :ref:`nRF Connect SDK <sdk_set_up>` (Zephyr RTOS and required modules).
The Aliro stack implements the Access Protocol logic, Aliro-specific cryptographic primitives, and communication with the User Device.
The interfaces layer connects the stack to the application through backends for NFC, Bluetooth LE, Ultra-Wideband (UWB), and crypto.
Custom backends can replace the default NFC and UWB implementations by implementing the provided APIs.

Aliro stack library
===================

The Aliro stack library files are located in :file:`lib/aliro`.
The stack can be built from source or linked as a precompiled binary, depending on the Kconfig configuration.

Application interface contract
==============================

The application implements the stack-defined interfaces in :file:`applications/*/src/aliro/interface_impl/`
and calls the Aliro stack public API to drive sessions and handle events.
For sequence diagrams and interaction flows between the application, Aliro stack, and transport backends,
see :ref:`aliro_application_interactions`.
