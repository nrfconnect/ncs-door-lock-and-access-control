#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library()

# The NfcTransport glue that connects the backend to the Aliro stack is always
# built. The backend that implements nfc_transport/nfc_transport_backend.h is
# selected by the DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND choice:
#  - RFAL (default): build the in-tree ST RFAL backend from rfal_backend/.
#  - EXTERNAL:       build no in-tree backend; an out-of-tree module defines
#                    the backend functions.
zephyr_library_sources(src/nfc_transport.cpp)

if(CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND_RFAL)
  add_subdirectory(rfal_backend)
elseif(CONFIG_DOOR_LOCK_ALIRO_NFC_TRANSPORT_BACKEND_EXTERNAL)
  # Intentionally left empty: the backend is provided out-of-tree.
endif()
