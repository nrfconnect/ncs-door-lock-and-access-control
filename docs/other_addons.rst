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

The Aliro UWB adapter is not yet available as a public add-on repository.
However, the UWB adapter library is included as part of |REPO_NAME| and can be found in the :file:`lib/qm35_uwb` directory.
For information on how to build it, see the :ref:`building_and_running` page.
