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
The binary is built from the Aliro add-on, which is a private module with restricted access.
See the following diagram for deployment of software components used by the |REPO_NAME|:

.. figure:: /images/aliro-add-ons.svg
   :scale: 100%
   :alt: Aliro add-ons deployment

   Aliro add-ons deployment

The Aliro UWB adapter is provided by the public `qm35-aliro-sdk <qm35-aliro-sdk_>`_ repository.
The repository is pulled into the workspace through the ``qm35-aliro-sdk`` west manifest group, which is disabled by default.
For information on how to build it, see the :ref:`aliro_matter_access_control_application` page.

The |REPO_NAME| also provides a vendor-neutral UWB platform interface under :file:`subsys/aliro/uwb/`.
The Qorvo QM35825 integration in :file:`subsys/aliro/uwb/qm35_impl/` is a reference port.
To integrate a different UWB chip, start from :file:`subsys/aliro/uwb/custom_impl/` and follow the :ref:`uwb_custom_integration` documentation page.
