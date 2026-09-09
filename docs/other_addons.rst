.. _other_addons:

Software components deployment
##############################

.. contents::
   :local:
   :depth: 2

This page details the deployment of the |REPO_NAME| and its dependencies.

Overview
********

The |REPO_NAME| functions as the |NCS| top-level add-on.
The top-level add-on leverages `other add-ons <ncs app index_>`_, which are typically configured as Zephyr modules, to enhance functionality.
All add-ons provide additional software deployed outside of the |NCS|.
Each of them operates with its own release cycle but is designed to work with specific versions of the |NCS|.
Dependencies between these modules and their specific revisions are managed through the :file:`west.yaml` file using `west, a Zephyr OS meta tool <west_>`_.

Aliro add-ons
*************

The |REPO_NAME| includes a binary library of the Aliro stack.
The binary is built from the source code, which is kept in a private repository with restricted access.
See the following diagram for deployment of software components used by the |REPO_NAME|:

.. figure:: /images/aliro-add-ons.svg
   :scale: 100%
   :alt: Aliro add-ons deployment

   Aliro add-ons deployment

The |REPO_NAME| provides a vendor-neutral UWB platform interface under :file:`subsys/aliro/uwb/`.
The core repository ships the generic ``UltraWideBand`` facade and an in-tree stub implementation in :file:`subsys/aliro/uwb/stub_impl/`.
A real UWB backend is provided by an external UWB provider module, which you can select through the ``DOOR_LOCK_ALIRO_UWB_IMPL`` Kconfig choice.
To integrate a UWB chip, follow the :ref:`uwb_custom_integration` documentation page.
