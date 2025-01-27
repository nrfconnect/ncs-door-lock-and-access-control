# About the RFAL layer

The driver utilizes the **RF Abstraction Layer (RFAL)**, which is designed to handle multiple drivers for specific ST NFC platforms. This abstraction layer simplifies the integration and management of different NFC drivers by providing a common interface, which ensures that the RFAL can effectively manage communication and features specific to given NFC module.

`PAL` directory contains the code that implements RFAL using Zephyr OS primitives.

`include/rfal_platform.h` file defines mapping between RFAL symbols and Zephyr specific implementation.

Currently, the solution supports driver for platform that uses **ST25R3911** transceiver.

## Integrating a New ST Driver

Follow the steps below to integrate a new ST driver into your project.

### Step 1: Prepare the Driver Files
1. Download the appropriate driver package for your NFC platform from [ST software page](https://www.st.com/en/embedded-software/st25-nfc-rfid-software/products.html).
2. Unzip the downloaded package and locate the folder containing the driver files.
3. Copy the folder with the driver files to `drivers/nfc/stm/RFAL/source`

**Note:** just in case some changes are made to the RFAL layer, you can copy just the `include` and `source` folders and place them in the `drivers/nfc/stm/RFAL` directory.

### Step 2: NFC Platform Integration
1. Create a shield for your ST NFC platform

    You need to create an appropriate shield to determine which ST NFC platform you use. In order to do this you have provide an appropriate Kconfig.shield in a dedicated folder in `/drivers/nfc/stm`.

    Here is an example of the code that maps the shield name into the corresponding Kconfig option:

        config X_NUCLEO_NFCYYYY
	        def_bool $(shields_list_contains,x_nucleo_nfcyyyy)
        
    Replace YYYY with the appropriate values for your platform.
    For example, if you are using X-NUCLEO-NFC08A1 your Kconfig.shield would look like this:

        config X_NUCLEO_NFC08A1
	        def_bool $(shields_list_contains,x_nucleo_nfc08a1)

2. Create a Kconfig for your ST NFC platform

    In order to do this you need to create a new Kconfig option in `/drivers/nfc/stm/Kconfig`. Here is an example:

        config ST25RXXXX_DRV
            bool "NFC ST25RXXXX driver"
            default y if X_NUCLEO_NFCYYYY
            select SPI

    Replace XXXX and YYYY with the appropriate values for your platform.
    For example, if you are using X-NUCLEO-NFC08A1, your platform driver is st25r3916B, and your Kconfig would look like this:

        config ST25R3916B_DRV
            bool "NFC ST25R3916B driver"
            default y if X_NUCLEO_NFC08A1
            select SPI

    You also need to provide a path to a Kconfig.shield file created in **point 1** in `drivers/nfc/stm/Kconfig`, Here is an example

        rsource "x_nucleo_nfcxxxx/Kconfig.shield"

3. Update the CmakeLists.txt file

    You need to add your NFC platform driver files to `drivers/nfc/stm/CmakeLists.txt` file, use Kconfig defined in **point 2**, Here's an example:

        if(CONFIG_ST25RXXXX_DRV)
            zephyr_library_compile_definitions(-DST25RXXXX) 
            set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25rXXXX)

    Replace XXXX with the appropriate value for your platform.
    For example, if your platform driver is st25r3916B, the CMakeLists entry would be:

        if(CONFIG_ST25R3916B_DRV)
            zephyr_library_compile_definitions(-DST25R3916B)
            set(ST_DRIVER_INC ${ST_RFAL_SRC_DIR}/st25r3916B)
        endif()

# Testing

After integrating your ST NFC platform driver, you can test it by running an NFC sample application. Navigate to the `drivers/samples/nfc_reader` directory in your project and build the application using the following command:

    west build -b board_name -p -- -DCONFIG_ST25RXXXX_DRV=y -DCONFIG_ST25R3911_DRV=n

where X_NUCLEO_NFCYYYY is shield which you have created in **point 1** of NFC Platform Integration section.

For example, if you created a shield for X-NUCLEO-NFC08A1, the command would be:

    west build -b nrf52840dk_nrf52840 -p -- -DCONFIG_ST25R3916B_DRV=y -DCONFIG_ST25R3911_DRV=n
