# RF Abstraction Layer overview

The driver uses the RF Abstraction Layer (RFAL) to handle multiple drivers for specific ST NFC platforms. This abstraction layer simplifies the integration and management of different NFC drivers by providing a common interface, which ensures effective communication and feature management for a given NFC module.

For more information about the RFAL, see the [ST RFAL user manual](https://www.st.com/resource/en/user_manual/um2890-rfnfc-abstraction-layer-rfal-stmicroelectronics.pdf).

## Structure and definitions

There are following components of the code structure and their roles within the RFAL implementation:

- `PAL` directory - Contains the code that implements RFAL using Zephyr OS primitives.
- `include/rfal_platform.h` file - Defines the mapping between RFAL symbols and Zephyr-specific implementation.

## Supported configuration

See the list of the NFC drivers currently supported by RFAL:

- ST25R3911
- ST25R3911B
- ST25R3916
- ST25R3916B
- ST25R100
- ST25R200
- ST25R300
- ST25R500

## Supported shields

See the list of the NFC shields currently supported by RFAL:
- [X-NUCLEO-NFC05A1](https://www.st.com/en/ecosystems/x-nucleo-nfc05a1.html)
- [X-NUCLEO-NFC08A1](https://www.st.com/en/ecosystems/x-nucleo-nfc08a1.html)
- [X-NUCLEO-NFC09A1](https://www.st.com/en/ecosystems/x-nucleo-nfc09a1.html)
- [X-NUCLEO-NFC12A1](https://www.st.com/en/ecosystems/x-nucleo-nfc12a1.html)

> Note: The STM NFC transceivers and shields supported in the nRF Connect Door Lock reference application may not match the full list supported natively by RFAL.

## Integrating a New ST Low Level Driver for NFC IC

Complete the following steps to integrate a new ST driver into your project.

> Note: All file paths used in the guide are relative to the location of this `README` file.

### 1. Preparing the driver files

1. Download a low-level driver package for your ST25R device. Open the [ST software page](https://www.st.com/en/embedded-software/st25-nfc-rfid-software/products.html), type `RFAL` in the search window, and select the appropriate version.

2. Unzip the downloaded package and locate the folder containing the driver files.

3. Copy the folder with the driver files to the `/RFAL/source` directory:

        ├── app
        │   └── src
        ├── docs
        ├── drivers
        │   ├── nfc
        │   │   └── stm
        │   │       ├── include
        │   │       ├── PAL
        │   │       └── RFAL
        │   │           ├── doc
        │   │           │   └── _htmresc
        │   │           ├── include
        │   │           └── source
        │   │               ├── st25r200
        │   │               ├── st25r3911
        │   │               └── st25r3916
        │   │               ├── st25r500
        |   |               └── **[NEW RFAL HAL]**
        │   └── samples
        │       └── nfc_reader
        │           ├── boards
        │           └── src
        ├── dts
        │   └── bindings
        └── zephyr


> Note: If there are updates to the RFAL layer, simply copy the include and source folders and place them in the `RFAL` directory.

### 2. Integrating NFC platform

1. Add configuration for your new ST NFC platform.

    First, you must add a new configuration option in the `Kconfig` file. See the following example:

        config ST25RXXXX_DRV
            bool "NFC ST25RXXXX driver"

    Replace `XXXX` with the appropriate values for your platform.
    For example, if you are using X-NUCLEO-NFC03A1, your platform driver is ST25R95, and your `Kconfig` should be as follows:

        config ST25R95_DRV
            bool "NFC ST25R95 driver"

2. Update the `CmakeLists.txt` file.

    Add your NFC platform driver directory to the `CmakeLists.txt` file using the `Kconfig` option defined in step 1:

        elseif(CONFIG_ST25RXXXX_DRV)
            zephyr_library_compile_definitions(-DST25RXXXX)
            set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25rXXXX)
        else()

    Replace `XXXX` with the appropriate value for your platform.
    For example, if your platform driver is ST25R95, the CMakeLists entry would be:

        elseif(CONFIG_ST25R95_DRV)
            zephyr_library_compile_definitions(-DST25R95)
            set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25r95)
        else()

# Testing

After integrating your ST NFC platform driver, you can test it by running the NFC sample application. Navigate to the `drivers/samples/nfc_reader` directory in your project and build the application using the following command:

    west build -b [board_name] -p -- -DCONFIG_NFC_DRIVER_STM=y -DCONFIG_ST25RXXXX_DRV=y

where CONFIG_ST25RXXXX_DRV is `Kconfig` which you have created in **step 1** of Integrating NFC platform section.

For example, if you created a `Kconfig` for X-NUCLEO-NFC03A1, the build command for `nrf52840dk_nrf52840` board command would be:

    west build -b nrf52840dk_nrf52840 -p -- -DCONFIG_NFC_DRIVER_STM=y -DCONFIG_ST25R95_DRV=y
