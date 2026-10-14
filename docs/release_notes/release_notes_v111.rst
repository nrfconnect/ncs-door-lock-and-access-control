.. _release_notes_v111:

Release notes for |REPO_NAME| v1.1.1
####################################

This is a bugfix release for v1.1.0.
It addresses UWB build and calibration issues, Matter lock event reporting, Zephyr module metadata, and adds Qorvo vendor1 extensions to the west manifest.

The following updates were introduced in this release.

* Fixed:

  * Missing ``MutexGuard`` namespace in the QM35 UWB implementation, which caused a compilation failure.
  * Incorrect ``UserIndex`` value reported in the Matter ``LockOperation`` event.
  * Missing Qorvo vendor1 extensions for QM35 UWB in the west manifest.
  * Swapped QM35825 2-port and 4-port UWB calibration data files.
  * Duplicated ``module.yaml`` and incorrect module name in ``module.yml``.
