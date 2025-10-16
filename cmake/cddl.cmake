#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

# Generate CBOR sources from CDDL files
# Usage: cddl_generate_cbor_sources(
#   MODULE_NAME <module_name>           # e.g., session_data
#   CDDL_FILES <file1 file2 file3>     # list of paths to .cddl files
#   ENCODE_OR_DECODE <encode|decode>    # whether to generate encode or decode
#   TYPES <type1 type2 type3>           # list of types to generate
#   DEFAULT_MAX_QTY <number>            # default maximum quantity
#   TARGET <target_name>                # target to link the library to
# )
function(cddl_generate_cbor_sources)
  set(options)
  set(oneValueArgs MODULE_NAME ENCODE_OR_DECODE TARGET DEFAULT_MAX_QTY)
  set(multiValueArgs CDDL_FILES TYPES)
  cmake_parse_arguments(CBOR "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT CBOR_MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is required for cddl_generate_cbor_sources")
  endif()
  if(NOT CBOR_CDDL_FILES)
    message(FATAL_ERROR "CDDL_FILES is required for cddl_generate_cbor_sources")
  endif()
  if(NOT CBOR_ENCODE_OR_DECODE)
    message(FATAL_ERROR "ENCODE_OR_DECODE is required for cddl_generate_cbor_sources")
  endif()
  if(NOT CBOR_TYPES)
    message(FATAL_ERROR "TYPES is required for cddl_generate_cbor_sources")
  endif()
  if(NOT CBOR_DEFAULT_MAX_QTY)
    set(CBOR_DEFAULT_MAX_QTY 1)
  endif()
  if(NOT CBOR_TARGET)
    message(FATAL_ERROR "TARGET is required for cddl_generate_cbor_sources")
  endif()

  # Validate ENCODE_OR_DECODE parameter
  if(NOT CBOR_ENCODE_OR_DECODE STREQUAL "encode" AND NOT CBOR_ENCODE_OR_DECODE STREQUAL "decode")
    message(FATAL_ERROR "ENCODE_OR_DECODE must be either 'encode' or 'decode'")
  endif()

  # Set up paths
  set(CBOR_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/cbor_${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE})
  set(OUTPUT_C_FILE ${CBOR_GENERATED_DIR}/src/${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE}.c)
  set(OUTPUT_H_FILE ${CBOR_GENERATED_DIR}/include/cbor/${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE}.h)
  set(OUTPUT_TYPES_H_FILE ${CBOR_GENERATED_DIR}/include/cbor/${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE}_types.h)

  # Build the zcbor command with separate -c arguments for each CDDL file
  set(ZCBOR_SCRIPT ${ZEPHYR_ZCBOR_MODULE_DIR}/zcbor/zcbor.py)
  set(ZCBOR_COMMAND ${PYTHON_EXECUTABLE} ${ZCBOR_SCRIPT} code)
  foreach(CDDL_FILE ${CBOR_CDDL_FILES})
    list(APPEND ZCBOR_COMMAND -c ${CDDL_FILE})
  endforeach()
  list(APPEND ZCBOR_COMMAND
    --${CBOR_ENCODE_OR_DECODE}
    --output-c ${OUTPUT_C_FILE}
    --output-h ${OUTPUT_H_FILE}
    --output-h-types ${OUTPUT_TYPES_H_FILE}
    --default-max-qty ${CBOR_DEFAULT_MAX_QTY}
    -t ${CBOR_TYPES}
  )

  # Generate the custom command
  add_custom_command(
    DEPENDS
      ${CBOR_CDDL_FILES}
    OUTPUT
      ${OUTPUT_C_FILE}
      ${OUTPUT_H_FILE}
      ${OUTPUT_TYPES_H_FILE}
    COMMAND
      ${ZCBOR_COMMAND}
      WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT
        "Generating CBOR ${CBOR_MODULE_NAME} ${CBOR_ENCODE_OR_DECODE} code"
  )

  # Create the library target
  add_library(cbor_${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE} OBJECT)
  target_link_libraries(cbor_${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE} PUBLIC zephyr_interface)
  target_include_directories(cbor_${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE} PUBLIC ${CBOR_GENERATED_DIR}/include)
  target_include_directories(cbor_${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE} PRIVATE ${CBOR_GENERATED_DIR}/include/cbor)
  target_sources(cbor_${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE} PRIVATE ${OUTPUT_C_FILE})
  target_link_libraries(${CBOR_TARGET} PRIVATE cbor_${CBOR_MODULE_NAME}_${CBOR_ENCODE_OR_DECODE})
endfunction()
