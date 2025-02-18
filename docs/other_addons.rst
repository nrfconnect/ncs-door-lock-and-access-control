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

|APP_NAME| utilizes the Aliro stack add-on to implement the Aliro protocol.
The Aliro stack is included as a private module, which means it has restricted access.
Additionally, it must be configured as a Zephyr module.

See the following diagram for deployment of software components used by |APP_NAME|:

.. figure:: /images/aliro-add-ons.svg
   :scale: 100%
   :alt: Aliro add-ons deployment

   Aliro add-ons deployment
