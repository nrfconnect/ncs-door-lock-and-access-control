.. _other_addons:

Software components deployment
##############################

This page details the deployment of the |APP_NAME| and its dependencies.

Overview
********

The |APP_NAME| functions as the nRF Connect top-level add-on.
The top-level add-on leverages other add-ons, which are typically configured as Zephyr modules, to enhance functionality.
All add-ons provide additional software deployed outside of the |NCS|.
Each of them operates with its own release cycle but is designed to work with specific versions of the |NCS|.
Dependencies between these modules and their specific revisions are managed through the :file:`west.yaml` file using `west, a Zephyr OS meta tool <west_>`_.

Aliro add-ons
*************

The |APP_NAME| includes a binary library of the Aliro stack.
The binary is built from the Aliro add-on, which is a private module with restricted access.

See the following diagram for deployment of software components used by the |APP_NAME|:

.. figure:: /images/aliro-add-ons.svg
   :scale: 100%
   :alt: Aliro add-ons deployment

   Aliro add-ons deployment
