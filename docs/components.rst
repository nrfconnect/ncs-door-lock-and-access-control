.. _components:

Components
##########

The |REPO_NAME| includes reusable software modules that implement product-facing behavior shared across the reference applications.
These components live in the :file:`subsys/` directory.
This section documents the most important modules, their configuration options, and how to adapt or replace them for your hardware.
For modules not covered here, see the corresponding Kconfig and public header files under :file:`subsys/` for options, dependencies, and usage.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   components/aliro_lock_sim.rst
   components/dfu_smp_service.rst
   components/nus_service.rst
