/*
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CHERRY_DEPRECATED
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ > 202301L) || \
	(defined(__cplusplus) && __cplusplus > 201402L)
#define CHERRY_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__) /* GCC or Clang */
#define CHERRY_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else /* Other */
#define CHERRY_DEPRECATED(msg)
#endif
#endif

#define CHERRY_NLOS_FP_ISSUE_MASK 1 << 0

#define CHERRY_NLOS_MASK CHERRY_NLOS_FP_ISSUE_MASK
#define CHERRY_IS_LOS(x) ((x && CHERRY_NLOS_MASK) == 0)

#define CHERRY_NLOS_NOT_SUPPORTED 255

/**
 * enum cherry_common_frame_status - Status of a UWB frame transfer
 */
enum cherry_common_frame_status {
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_OK: Success.
	 */
	CHERRY_COMMON_FRAME_STATUS_OK,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_UNKNOWN: Unknown failure.
	 */
	CHERRY_COMMON_FRAME_STATUS_UNKNOWN,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_TX_FAILED: Failed to transmit the UWB
	 * frame.
	 */
	CHERRY_COMMON_FRAME_STATUS_TX_FAILED = 0x20,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_RX_TIMEOUT: Timeout to receive the UWB frame.
	 */
	CHERRY_COMMON_FRAME_STATUS_RX_TIMEOUT,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_RX_PHY_DEC_FAILED: UWB packet channel
	 * decoding error.
	 */
	CHERRY_COMMON_FRAME_STATUS_RX_PHY_DEC_FAILED,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_RX_PHY_TOA_FAILED: Failed to detect time of
	 * arrival of the UWB frame
	 */
	CHERRY_COMMON_FRAME_STATUS_RX_PHY_TOA_FAILED,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_RX_PHY_STS_FAILED: UWB packet STS segment
	 * mismatch
	 */
	CHERRY_COMMON_FRAME_STATUS_RX_PHY_STS_FAILED,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_RX_MAC_DEC_FAILED: MAC CRC or syntax error
	 */
	CHERRY_COMMON_FRAME_STATUS_RX_MAC_DEC_FAILED,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_RX_MAC_IE_DEC_FAILED: IE syntax error
	 */
	CHERRY_COMMON_FRAME_STATUS_RX_MAC_IE_DEC_FAILED,
	/**
	 * @CHERRY_COMMON_FRAME_STATUS_RX_MAC_IE_MISSING: Expected IE is missing
	 * in the packet
	 */
	CHERRY_COMMON_FRAME_STATUS_RX_MAC_IE_MISSING,
};
/**
 * enum cherry_common_rframe_config - STS Packet Configuration.
 *
 * @CHERRY_COMMON_RFRAME_CONFIG_SP0: STS Packet Configuration 0 (SP0)
 * (Reserved value for test purpose).
 * @CHERRY_COMMON_RFRAME_CONFIG_SP1: STS Packet Configuration 1 (SP1).
 * @CHERRY_COMMON_RFRAME_CONFIG_SP3: STS Packet Configuration 3 (SP3).
 * @CHERRY_COMMON_RFRAME_CONFIG_DEFAULT: STS Packet Configuration 3 (SP3).
 */
enum cherry_common_rframe_config {
	CHERRY_COMMON_RFRAME_CONFIG_SP0 = 0x00,
	CHERRY_COMMON_RFRAME_CONFIG_SP1 = 0x01,
	CHERRY_COMMON_RFRAME_CONFIG_SP3 = 0x03,
	CHERRY_COMMON_RFRAME_CONFIG_DEFAULT = CHERRY_COMMON_RFRAME_CONFIG_SP3,
};

/**
 * enum cherry_common_prf_mode - Mean Pulse Repetition Frequency.
 *
 * @CHERRY_COMMON_PRF_MODE_BPRF_62_4M: 62.4 MHz PRF in BPRF mode.
 * @CHERRY_COMMON_PRF_MODE_DEFAULT: 62.4 MHz PRF in BPRF mode.
 * @CHERRY_COMMON_PRF_MODE_HPRF_124_8M: 124.8 MHz PRF in HPRF mode.
 * @CHERRY_COMMON_PRF_MODE_HPRF_249_6M: 249.6 MHz PRF in HPRF mode.
 * @CHERRY_COMMON_PRF_MODE_HPRFG_249_6M: 249.6 MHz PRF in HPRF mode.
 */
enum cherry_common_prf_mode {
	CHERRY_COMMON_PRF_MODE_BPRF_62_4M = 0x00,
	CHERRY_COMMON_PRF_MODE_DEFAULT = CHERRY_COMMON_PRF_MODE_BPRF_62_4M,
	CHERRY_COMMON_PRF_MODE_HPRF_124_8M,
	CHERRY_COMMON_PRF_MODE_HPRF_249_6M,
	CHERRY_COMMON_PRF_MODE_HPRFG_249_6M =
		CHERRY_COMMON_PRF_MODE_HPRF_249_6M,
};

/**
 * enum cherry_common_preamble_duration - Preamble Symbol Repetitions (PSR).
 *
 * @CHERRY_COMMON_PREAMBLE_DURATION_32: 32 symbols.
 * @CHERRY_COMMON_PREAMBLE_DURATION_64: 64 symbols.
 * @CHERRY_COMMON_PREAMBLE_DURATION_DEFAULT: 64 symbols.
 */
enum cherry_common_preamble_duration {
	CHERRY_COMMON_PREAMBLE_DURATION_32 = 0x00,
	CHERRY_COMMON_PREAMBLE_DURATION_64,
	CHERRY_COMMON_PREAMBLE_DURATION_DEFAULT =
		CHERRY_COMMON_PREAMBLE_DURATION_64,
};

#define CHERRY_COMMON_PREAMBLE_CODE_INDEX_DEFAULT 10

/**
 * enum cherry_common_sts_length - Number of symbols in the STS segment.
 *
 * @CHERRY_COMMON_STS_LENGTH_32: 32 symbols.
 * @CHERRY_COMMON_STS_LENGTH_64: 64 symbols.
 * @CHERRY_COMMON_STS_LENGTH_DEFAULT: 64 symbols.
 * @CHERRY_COMMON_STS_LENGTH_128: 128 symbols.
 * @CHERRY_COMMON_STS_LENGTH_NA: Not set.
 */
enum cherry_common_sts_length {
	CHERRY_COMMON_STS_LENGTH_32 = 0x00,
	CHERRY_COMMON_STS_LENGTH_64,
	CHERRY_COMMON_STS_LENGTH_DEFAULT = CHERRY_COMMON_STS_LENGTH_64,
	CHERRY_COMMON_STS_LENGTH_128,
	CHERRY_COMMON_STS_LENGTH_NA = 0xFF,
};

/**
 * enum cherry_common_psdu_data_rate - Data rate for PHY service Data Unit.
 *
 * @CHERRY_COMMON_PSDU_DATA_RATE_6_81M: 6.81 Mbps.
 * @CHERRY_COMMON_PSDU_DATA_RATE_DEFAULT: 6.81 Mbps.
 * @CHERRY_COMMON_PSDU_DATA_RATE_7_80M: 7.80 Mbps.
 * @CHERRY_COMMON_PSDU_DATA_RATE_27_2M: 27.2 Mbps.
 * @CHERRY_COMMON_PSDU_DATA_RATE_31_2M: 31.2 Mbps.
 * @CHERRY_COMMON_PSDU_DATA_RATE_850K: 850 Kbps.
 * @CHERRY_COMMON_PSDU_DATA_RATE_NA: Not set.
 */
enum cherry_common_psdu_data_rate {
	CHERRY_COMMON_PSDU_DATA_RATE_6_81M = 0x00,
	CHERRY_COMMON_PSDU_DATA_RATE_DEFAULT =
		CHERRY_COMMON_PSDU_DATA_RATE_6_81M,
	CHERRY_COMMON_PSDU_DATA_RATE_7_80M,
	CHERRY_COMMON_PSDU_DATA_RATE_27_2M,
	CHERRY_COMMON_PSDU_DATA_RATE_31_2M,
	CHERRY_COMMON_PSDU_DATA_RATE_850K,
	CHERRY_COMMON_PSDU_DATA_RATE_NA = 0xFF,
};

/**
 * enum cherry_common_mac_fcs_type - Type of FCS used by tags.
 *
 * @CHERRY_COMMON_MAC_FCS_TYPE_CRC16: 16 bits CRC
 * @CHERRY_COMMON_MAC_FCS_TYPE_CRC32: 32 bits CRC
 */
enum cherry_common_mac_fcs_type {
	CHERRY_COMMON_MAC_FCS_TYPE_CRC16 = 0,
	CHERRY_COMMON_MAC_FCS_TYPE_CRC32 = 1,
};

/**
 * struct cherry_common_diag_cfg - Define which kind of diagnostic have to be
 * reported.
 */
struct cherry_common_diag_cfg {
	/**
	 * @extra_status: Enable the reporting of extra status information per frame. This flags
	 * is considered enabled as soon as one of the other diagnostic config is enabled.
	 * It allows to enable a minimal diagnostics report event with the @msg_ig, @action,
	 * @antenna_set and @extra_status fields of struct cherry_common_diag_frame.
	 */
	bool extra_status;
	/**
	 * @aoa: Enable the reporting of AoA measurement metrics of an antenna pair.
	 * When enabled, an array of struct cherry_common_aoa_measurement is reported on diagnostics
	 * report event for each ranging frame received.
	 */
	bool aoa;
	/**
	 * @cfo: Enable the reporting of Clock Frequency Offset.
	 *
	 */
	bool cfo;
	/**
	 * @emitter_addr: Enable the reporting of Frame emitter short address.
	 *
	 */
	bool emitter_addr;
	/**
	 * @seg_metrics: Enable the reporting of received frame metrics.
	 * When enabled, an array of struct cherry_common_segment_metrics is reported on diagnostics
	 * report event for each frame received.
	 *
	 */
	bool seg_metrics;
	/**
	 * @cirs: Enable the reporting of measured Channel Impulse Response.
	 * When enabled, an array of struct cherry_common_cir is reported on diagnostics
	 * report event for each ranging frame received.
	 */
	bool cirs;
	/**
	 * @timestamp: Enable the reporting of timestamp metrics group in Segment Metrics field.
	 */
	bool timestamp;
	/**
	 * @rssi: Enable the reporting of rssi metrics group in Segment Metrics field.
	 */
	bool rssi;
	/**
	 * @rx_path: Enable the reporting of Rx path metrics group in Segment Metrics field.
	 */
	bool rx_path;
	/**
	 * @rx_debugging: Enable the reporting of Rx debugging metrics group in Segment Metrics field.
	 */
	bool rx_debugging;
	/**
	 * @fira_msg_id: Enable the reporting of FiRa message Id.
	 */
	bool fira_msg_id;
	/**
	 * @cir_windows_size: Size of CIR window to set for diagnostics.
	 */
	uint8_t cir_window_size;
	/**
	 * @cir_window_fp_offset: Set CIR window's FP offset for diagnostics.
	 */
	uint8_t cir_window_fp_offset;
};

/**
 * struct cherry_common_cir - Channel Impulse Response measured metrics.
 */
struct cherry_common_cir {
	/**
	 * @receiver_segment: The receiver and the frame segment the CIR has been computed.
	 *
	 * Formatted as:
	 *   b2-b0: Id of the segment (IPATOV, STS0, STS1, STS2, STS3)
	 *   b3: Primary/Secondary indicator
	 *   b7-b4: Id of the receiver from 0x0 to 0xF
	 */
	uint8_t receiver_segment;
	/**
	 * @fpath_tap_offset: Offset of the first path tap.
	 */
	int16_t fpath_tap_offset;
	/**
	 * @tap_size: Size of one tap in bytes.
	 */
	uint8_t tap_size;
	/**
	 * @n_taps: Number of taps.
	 */
	unsigned int n_taps;
	/**
	 * @taps: raw taps array of n_taps * tap_size.
	 */
	uint8_t *taps;
};

/**
 * struct cherry_common_aoa_measurement - measurement result for a pair of antennas
 */
struct cherry_common_aoa_measurement {
	/**
	 * @tdoa: Measured Time Difference of Arrival between 2 antennas.
	 * In ranging count time unit (16.65 pico seconds).
	 */
	int16_t tdoa;
	/**
	 * @pdoa: Computed Phase Difference of Arrival between 2 antennas.
	 * Signed Q4.11 fixed point format in radian.
	 */
	int16_t pdoa;
	/**
	 * @aoa: Computed Angle Difference of Arrival between 2 antennas.
	 * Signed Q4.11 fixed point format in radian.
	 */
	int16_t aoa;
	/**
	 * @fom: Figure of Merit associated to the AoA in percent.
	 * With 0 meaning invalid measure and 255 for 100% of confidence.
	 */
	uint8_t fom;
	/**
	 * @type: AoA axis type.
	 * Related to antenna configuration into the calibration.
	 *
	 *   0 for X axis
	 *   1 for Y axis
	 *   2 for Z axis
	 */
	uint8_t type;
};

/**
 * struct cherry_common_segment_metrics - Metrics measured for a received frame segment
 */
struct cherry_common_segment_metrics {
	/**
	 * @noise_value: The RF noise floor value in dBm measured during reception of the whole
	 * segment.
	 */
	int16_t noise_value;
	/**
	 * @rsl_q8: The absolute value in dBm of the RSL measured on the whole segment.
	 * Unsigned Q8.8 fixed point format in dBm.
	 */
	uint16_t rsl_q8;
	/**
	 * @fp_index: First Path index.
	 */
	uint16_t fp_index;
	/**
	 * @fp_rsl_q8: The absolute value in dBm of the RSL of the sample considered as the first
	 * path of the segment.
	 * Unsigned Q8.8 fixed point format in dBm.
	 */
	uint16_t fp_rsl_q8;
	/**
	 * @fp_ns_q6: First Path offset in nanosecond compare to the first CIR.
	 * Unsigned Q10.6 fixed point format in nanosecond.
	 */
	uint16_t fp_ns_q6;
	/**
	 * @pp_index: Peak Path index.
	 */
	uint16_t pp_index;
	/**
	 * @pp_rsl_q8: The absolute value in dBm of the RSL of the sample considered as the peak
	 * path of the segment.
	 * Unsigned Q8.8 fixed point format in dBm.
	 */
	uint16_t pp_rsl_q8;
	/**
	 * @pp_ns_q6: The position in nanoseconds of the Peak path from the start of the CIR for the
	 * segment.
	 * Unsigned Q10.6 fixed point format in nanosecond.
	 */
	uint16_t pp_ns_q6;
	/**
	 * @receiver_segment: The receiver and the frame segment the CIR has been computed.
	 *
	 * Formatted as:
	 *   b2-b0: Id of the segment (IPATOV, STS0, STS1, STS2, STS3)
	 *   b3: Primary/Secondary indicator
	 *   b7-b4: Id of the receiver from 0x0 to 0xF
	 */
	uint8_t receiver_segment;
	/**
	 * @control: Control field for present groups.
	 */
	uint8_t control;
	/**
	 * @timestamp: Timestamp in RCTUs.
	 */
	uint32_t timestamp;
	/**
	 * @early_fp_index: Early first path index.
	 */
	uint16_t early_fp_index;
	/**
	 * @early_fp_ns_q6: Early first path position in ns as Q6 real.
	 */
	uint16_t early_fp_ns_q6;
	/**
	 * @dgc_decision: DGC decision level [0-7].
	 */
	uint8_t dgc_decision;
	/**
	 * @fp_noise_threshold: First path noise threshold.
	 */
	uint32_t fp_noise_threshold;
	/**
	 * @fp_confidence: First path confidence level [0-8].
	 */
	uint8_t fp_confidence;
};

/**
 * struct cherry_common_diag_frame - Diagnostics data of one frame
 */
struct cherry_common_diag_frame {
	/**
	 * @seg_metrics: Array of segment's metrics.
	 * Available only when @seg_metrics is set to true in the diagnostic config, otherwise the
	 * pointer is set to NULL.
	 */
	struct cherry_common_segment_metrics *seg_metrics;
	/**
	 * @aoas: Array of AoA metrics.
	 * Available only when @aoa is set to true in the diagnostic config, otherwise the pointer is
	 * set to NULL.
	 */
	struct cherry_common_aoa_measurement *aoas;
	/**
	 * @cirs: Array of CIRs.
	 * Available only when @cirs is set to true in the diagnostic config, otherwise the pointer
	 * is set to NULL.
	 */
	struct cherry_common_cir *cirs;
	/**
	 * @cfo_q26: Clock Frequency Offset in Q26.
	 * Available only when @cfo is set to true in the diagnostic config, otherwise the value is
	 * set to 0.
	 */
	int32_t cfo_q26;
	/**
	 * @emitter_short_addr: Short address of the frame emitter.
	 * Available only when @emitter_addr is set to true in the diagnostic config, otherwise the
	 * value is set to 0.
	 */
	uint16_t emitter_short_addr;
	/**
	 * @extra_status: Bit field of extra status.
	 */
	uint16_t extra_status;
	/**
	 * @n_seg_metrics: Number of element into @seg_metrics array.
	 */
	unsigned int n_seg_metrics;
	/**
	 * @n_aoa: Number of element into @aoas array.
	 */
	unsigned int n_aoa;
	/**
	 * @n_cir: number of elements into @cirs array.
	 */
	unsigned int n_cir;
	/**
	 * @cfo_present: True if @cfo_q26 is filled.
	 */
	bool cfo_present;
	/**
	 * @emitter_short_addr_present: True if @emitter_short_addr is set.
	 */
	bool emitter_short_addr_present;
	/**
	 * @extra_status_present: True if @extra_status is set.
	 */
	bool extra_status_present;
	/**
	 * @timestamp_confidence_present: Whether timestamp confidence is present.

	 */
	bool timestamp_confidence_present;
	/**
	 * @fira_message_id_present: Whether FiRa message ID is present.

	 */
	bool fira_message_id_present;
	/**
	 * @msg_id: Message ID of the frame.
	 */
	uint8_t msg_id;
	/**
	 * @timestamp_confidence: Timestamp confidence [0-255].

	 */
	uint8_t timestamp_confidence;
	/**
	 * @fira_message_id: FiRa message ID.

	 */
	uint8_t fira_message_id;
	/**
	 * @action: 0 For RX and 1 for TX.
	 */
	uint8_t action;
	/**
	 * @antenna_set: ID of the antenna_set.
	 */
	unsigned int antenna_set;
};

/**
 * struct cherry_common_diag_report - Generic data format for
 * Diagnostics report event.
 */
struct cherry_common_diag_report {
	/**
	 * @session: Session context.
	 */
	void *session;
	/**
	 * @sequence_number: Session notification counter.
	 */
	uint32_t sequence_number;
	/**
	 * @n_frame_report: Number of frame into the frame_report table
	 * table.
	 */
	unsigned int n_frame_report;
	/**
	 * @frame_report: Diagnostic information per frame.
	 */
	struct cherry_common_diag_frame frame_report[];
};

/**
 * struct cherry_common_addr - Address of devices following 802.15.4 format.
 */
struct cherry_common_addr {
	/**
	 * @val: address value.
	 */
	uint64_t val;
};

/**
 * CHERRY_COMMON_ADDR_SHORT() - Return an address structure, from short address.
 * @short_addr: Short address.
 *
 * This is usable in structure initialization.
 *
 * Returns: Address structure.
 */
#define CHERRY_COMMON_ADDR_SHORT(short_addr) \
	((struct cherry_common_addr){ .val = (short_addr) })

/**
 * CHERRY_COMMON_ADDR_EXTENDED() - Return an address structure, from extended address.
 * @extended_addr: Extended address.
 *
 * This is usable in structure initialization.
 *
 * Returns: Address structure.
 */
#define CHERRY_COMMON_ADDR_EXTENDED(extended_addr) \
	((struct cherry_common_addr){ .val = (extended_addr) })

/**
 * cherry_common_addr_is_short() - Test whether an address is short.
 * @addr: Short or extended address.
 *
 * Returns: True if address is a short address.
 */
static inline bool cherry_common_addr_is_short(struct cherry_common_addr addr)
{
	return addr.val >> 16 == 0;
}

/**
 * cherry_common_addr_is_extended() - Test whether an address is extended.
 * @addr: Short or extended address.
 *
 * Returns: True if address is an extended address.
 */
static inline bool
cherry_common_addr_is_extended(struct cherry_common_addr addr)
{
	return addr.val >> 16 != 0;
}

/**
 * cherry_common_addr_get_short() - Get short address from address structure.
 * @addr: Address structure.
 *
 * Do not call this if the address is not a short address.
 *
 * Returns: Short address.
 */
static inline uint16_t
cherry_common_addr_get_short(struct cherry_common_addr addr)
{
	return (uint16_t)addr.val;
}

/**
 * cherry_common_addr_get_extended() - Get extended address from address structure.
 * @addr: Address structure.
 *
 * Do not call this if the address is not an extended address.
 *
 * Returns: Extended address.
 */
static inline uint64_t
cherry_common_addr_get_extended(struct cherry_common_addr addr)
{
	return addr.val;
}

/**
 * cherry_common_pdoa_to_double() - Convert PDOA from Q4.11 fixed point format to double.
 * @pdoa: PDOA value in Q4.11 fixed point format in radians.
 *
 * Converts the Phase Difference of Arrival value from Q4.11 fixed point format to a
 * double precision floating point value in radians.
 *
 * In Q4.11 format:
 * - 4 bits represent the integer part (including sign bit)
 * - 11 bits represent the fractional part
 *
 * Returns: PDOA value in radians as double.
 */
static inline double cherry_common_pdoa_to_double(int16_t pdoa)
{
	/* Check if the number is negative. */
	int32_t value = pdoa;
	int32_t is_negative = value & 0x8000; /* Check sign bit (bit 15) */
	double result;

	/* If negative, convert to positive using two's complement. */
	if (is_negative) {
		value = ~value + 1; /* Two's complement */
	}

	/* Convert to floating-point by dividing by 2^11 */
	result = value / (double)(1 << 11);

	/* Apply the sign. */
	if (is_negative) {
		result = -result;
	}

	return result;
}

#ifdef __cplusplus
}
#endif
