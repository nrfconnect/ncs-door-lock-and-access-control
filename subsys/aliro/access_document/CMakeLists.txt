#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

include(cddl)

cddl_generate_cbor_sources(
  MODULE_NAME access_document
  CDDL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/access_document.cddl
  ENCODE_OR_DECODE decode
  TYPES AccessData
  DEFAULT_MAX_QTY 3
  TARGET app
)
