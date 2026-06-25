/*
 * Header file for chip calibration.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-Qorvo-SDK-1
 */

#pragma once

#include <cherry/cherry.h>

/* Modified calibration based on jolie_quad_GSavg_27032025.json. */
static const uint8_t qm35825_calib_2port_ant_pair_0_ant_path[] = { 0x03, 0x01 };
static const uint8_t qm35825_calib_2port_ant_pair_1_ant_path[] = { 0x02, 0x01 };
static const uint8_t qm35825_calib_2port_ant_set0_tx_ant_paths[] = { 0x00,
								    0xFF };
static const uint8_t qm35825_calib_2port_ant_set3_tx_ant_paths[] = { 0x00,
								    0xFF };
/* ant_pair1 only: ant2 (ANT3) + ant1 (ANT2) — PDOA pair for 2-port antenna. */
static const uint8_t qm35825_calib_2port_ant_set0_rx_ants[] = { 0x01, 0xFF,
							       0xFF };
static const uint8_t qm35825_calib_2port_ant_set3_rx_ants[] = { 0x02, 0xFF,
							       0xFF };

static const uint8_t qm35825_calib_2port_ant_set4_tx_ant_paths[] = { 0x06,
								    0xFF };
static const uint8_t qm35825_calib_2port_ant_set4_rx_ants[] = { 0x04, 0xFF,
							       0xFF };
static const uint8_t qm35825_calib_2port_ant_set5_tx_ant_paths[] = { 0x07,
								    0xFF };
static const uint8_t qm35825_calib_2port_ant_set5_rx_ants[] = { 0x05, 0xFF,
							       0xFF };

static const uint16_t qm35825_calib_2port_pdoa_lut0_ch5[][2] = {
	{ 0xEB2D, 0xF36F }, /* original: -2.602930, -1.570796 */
	{ 0xEB58, 0xF445 }, /* original: -2.582271, -1.466077 */
	{ 0xEBC3, 0xF51C }, /* original: -2.529768, -1.361357 */
	{ 0xEC68, 0xF5F2 }, /* original: -2.449346, -1.256637 */
	{ 0xED2E, 0xF6C9 }, /* original: -2.352646, -1.151917 */
	{ 0xEE0E, 0xF79F }, /* original: -2.243066, -1.047198 */
	{ 0xEF15, 0xF876 }, /* original: -2.114530, -0.942478 */
	{ 0xF040, 0xF94C }, /* original: -1.968700, -0.837758 */
	{ 0xF197, 0xFA23 }, /* original: -1.801494, -0.733038 */
	{ 0xF324, 0xFAF9 }, /* original: -1.607584, -0.628319 */
	{ 0xF4E4, 0xFBD0 }, /* original: -1.388774, -0.523599 */
	{ 0xF6CE, 0xFCA6 }, /* original: -1.149404, -0.418879 */
	{ 0xF8D0, 0xFD7D }, /* original: -0.898402, -0.314159 */
	{ 0xFAF7, 0xFE53 }, /* original: -0.629632, -0.209440 */
	{ 0xFD4F, 0xFF2A }, /* original: -0.336526, -0.104720 */
	{ 0xFFE3, 0x0000 }, /* original: -0.014023, 0.000000 */
	{ 0x0285, 0x00D6 }, /* original: 0.314980, 0.104720 */
	{ 0x0512, 0x01AD }, /* original: 0.633580, 0.209440 */
	{ 0x077E, 0x0283 }, /* original: 0.936523, 0.314159 */
	{ 0x09C1, 0x035A }, /* original: 1.219335, 0.418879 */
	{ 0x0BD7, 0x0430 }, /* original: 1.480114, 0.523599 */
	{ 0x0DBC, 0x0507 }, /* original: 1.716937, 0.628319 */
	{ 0x0F73, 0x05DD }, /* original: 1.931293, 0.733038 */
	{ 0x1121, 0x06B4 }, /* original: 2.141143, 0.837758 */
	{ 0x12D0, 0x078A }, /* original: 2.351714, 0.942478 */
	{ 0x145E, 0x0861 }, /* original: 2.546088, 1.047198 */
	{ 0x15AE, 0x0937 }, /* original: 2.709871, 1.151917 */
	{ 0x169B, 0x0A0E }, /* original: 2.825772, 1.256637 */
	{ 0x176A, 0x0AE4 }, /* original: 2.926944, 1.361357 */
	{ 0x1845, 0x0BBB }, /* original: 3.033820, 1.466077 */
	{ 0x1963, 0x0C91 }, /* original: 3.173142, 1.570796 */
};

static const uint16_t qm35825_calib_2port_pdoa_lut1_ch9[][2] = {
	{ 0xE161, 0xF36F }, /* original: -3.827516, -1.570796 */
	{ 0xE46C, 0xF445 }, /* original: -3.447385, -1.466077 */
	{ 0xE740, 0xF51C }, /* original: -3.093565, -1.361357 */
	{ 0xE9C2, 0xF5F2 }, /* original: -2.780069, -1.256637 */
	{ 0xEC30, 0xF6C9 }, /* original: -2.476374, -1.151917 */
	{ 0xEEB5, 0xF79F }, /* original: -2.161623, -1.047198 */
	{ 0xF148, 0xF876 }, /* original: -1.839775, -0.942478 */
	{ 0xF382, 0xF94C }, /* original: -1.561503, -0.837758 */
	{ 0xF516, 0xFA23 }, /* original: -1.364230, -0.733038 */
	{ 0xF5FA, 0xFAF9 }, /* original: -1.253113, -0.628319 */
	{ 0xF68C, 0xFBD0 }, /* original: -1.181710, -0.523599 */
	{ 0xF795, 0xFCA6 }, /* original: -1.052401, -0.418879 */
	{ 0xF94A, 0xFD7D }, /* original: -0.838672, -0.314159 */
	{ 0xFB7D, 0xFE53 }, /* original: -0.563794, -0.209440 */
	{ 0xFDCE, 0xFF2A }, /* original: -0.274634, -0.104720 */
	{ 0xFFEE, 0x0000 }, /* original: -0.008756, 0.000000 */
	{ 0x01B5, 0x00D6 }, /* original: 0.213468, 0.104720 */
	{ 0x0347, 0x01AD }, /* original: 0.409443, 0.209440 */
	{ 0x04B6, 0x0283 }, /* original: 0.588916, 0.314159 */
	{ 0x062C, 0x035A }, /* original: 0.771504, 0.418879 */
	{ 0x07E2, 0x0430 }, /* original: 0.985169, 0.523599 */
	{ 0x09FE, 0x0507 }, /* original: 1.248879, 0.628319 */
	{ 0x0C45, 0x05DD }, /* original: 1.533630, 0.733038 */
	{ 0x0E72, 0x06B4 }, /* original: 1.805584, 0.837758 */
	{ 0x1059, 0x078A }, /* original: 2.043332, 0.942478 */
	{ 0x11E6, 0x0861 }, /* original: 2.237329, 1.047198 */
	{ 0x134C, 0x0937 }, /* original: 2.412205, 1.151917 */
	{ 0x14AE, 0x0A0E }, /* original: 2.585158, 1.256637 */
	{ 0x1608, 0x0AE4 }, /* original: 2.754007, 1.361357 */
	{ 0x1733, 0x0BBB }, /* original: 2.899662, 1.466077 */
	{ 0x1814, 0x0C91 }, /* original: 3.009969, 1.570796 */
};

static const uint16_t qm35825_calib_2port_pdoa_lut2_ch5[][2] = {
	{ 0xE9A9, 0xF36F }, /* original: -2.792527, -1.570796 */
	{ 0xEB3D, 0xF445 }, /* original: -2.595256, -1.466077 */
	{ 0xEC06, 0xF51C }, /* original: -2.497197, -1.361357 */
	{ 0xEC48, 0xF5F2 }, /* original: -2.465013, -1.256637 */
	{ 0xECE2, 0xF6C9 }, /* original: -2.389712, -1.151917 */
	{ 0xEDBD, 0xF79F }, /* original: -2.282880, -1.047198 */
	{ 0xEED4, 0xF876 }, /* original: -2.146335, -0.942478 */
	{ 0xF06B, 0xF94C }, /* original: -1.947676, -0.837758 */
	{ 0xF2BF, 0xFA23 }, /* original: -1.656585, -0.733038 */
	{ 0xF482, 0xFAF9 }, /* original: -1.436441, -0.628319 */
	{ 0xF5D0, 0xFBD0 }, /* original: -1.273355, -0.523599 */
	{ 0xF7A3, 0xFCA6 }, /* original: -1.045348, -0.418879 */
	{ 0xF91A, 0xFD7D }, /* original: -0.862066, -0.314159 */
	{ 0xFB62, 0xFE53 }, /* original: -0.577365, -0.209440 */
	{ 0xFE0D, 0xFF2A }, /* original: -0.243863, -0.104720 */
	{ 0xFFEE, 0x0000 }, /* original: -0.008900, 0.000000 */
	{ 0x034C, 0x00D6 }, /* original: 0.412168, 0.104720 */
	{ 0x067D, 0x01AD }, /* original: 0.811005, 0.209440 */
	{ 0x08AF, 0x0283 }, /* original: 1.085629, 0.314159 */
	{ 0x0B49, 0x035A }, /* original: 1.410721, 0.418879 */
	{ 0x0D96, 0x0430 }, /* original: 1.698024, 0.523599 */
	{ 0x0F20, 0x0507 }, /* original: 1.890397, 0.628319 */
	{ 0x10B5, 0x05DD }, /* original: 2.088518, 0.733038 */
	{ 0x1220, 0x06B4 }, /* original: 2.265577, 0.837758 */
	{ 0x132D, 0x078A }, /* original: 2.397055, 0.942478 */
	{ 0x13E2, 0x0861 }, /* original: 2.485442, 1.047198 */
	{ 0x1482, 0x0937 }, /* original: 2.563443, 1.151917 */
	{ 0x1500, 0x0A0E }, /* original: 2.624997, 1.256637 */
	{ 0x1531, 0x0AE4 }, /* original: 2.649017, 1.361357 */
	{ 0x158F, 0x0BBB }, /* original: 2.694788, 1.466077 */
	{ 0x1657, 0x0C91 }, /* original: 2.792527, 1.570796 */
};

static const uint16_t qm35825_calib_2port_pdoa_lut3_ch9[][2] = {
	{ 0xE843, 0xF36F }, /* original: -2.967060, -1.570796 */
	{ 0xE91A, 0xF445 }, /* original: -2.862340, -1.466077 */
	{ 0xEA40, 0xF51C }, /* original: -2.718937, -1.361357 */
	{ 0xEB5B, 0xF5F2 }, /* original: -2.580806, -1.256637 */
	{ 0xEBD1, 0xF6C9 }, /* original: -2.523085, -1.151917 */
	{ 0xEC6D, 0xF79F }, /* original: -2.446824, -1.047198 */
	{ 0xEE54, 0xF876 }, /* original: -2.209184, -0.942478 */
	{ 0xF17A, 0xF94C }, /* original: -1.815553, -0.837758 */
	{ 0xF490, 0xFA23 }, /* original: -1.429622, -0.733038 */
	{ 0xF6DB, 0xFAF9 }, /* original: -1.143278, -0.628319 */
	{ 0xF7D0, 0xFBD0 }, /* original: -1.023196, -0.523599 */
	{ 0xF89B, 0xFCA6 }, /* original: -0.924179, -0.418879 */
	{ 0xFA09, 0xFD7D }, /* original: -0.745822, -0.314159 */
	{ 0xFB73, 0xFE53 }, /* original: -0.568702, -0.209440 */
	{ 0xFDBC, 0xFF2A }, /* original: -0.283155, -0.104720 */
	{ 0xFFE6, 0x0000 }, /* original: -0.012917, 0.000000 */
	{ 0x0291, 0x00D6 }, /* original: 0.320914, 0.104720 */
	{ 0x0509, 0x01AD }, /* original: 0.629215, 0.209440 */
	{ 0x079B, 0x0283 }, /* original: 0.950551, 0.314159 */
	{ 0x09F8, 0x035A }, /* original: 1.246241, 0.418879 */
	{ 0x0C23, 0x0430 }, /* original: 1.517158, 0.523599 */
	{ 0x0ED0, 0x0507 }, /* original: 1.851507, 0.628319 */
	{ 0x111A, 0x05DD }, /* original: 2.137758, 0.733038 */
	{ 0x132E, 0x06B4 }, /* original: 2.397456, 0.837758 */
	{ 0x14BD, 0x078A }, /* original: 2.592345, 0.942478 */
	{ 0x15D6, 0x0861 }, /* original: 2.729294, 1.047198 */
	{ 0x1714, 0x0937 }, /* original: 2.884840, 1.151917 */
	{ 0x17FD, 0x0A0E }, /* original: 2.998408, 1.256637 */
	{ 0x1893, 0x0AE4 }, /* original: 3.071675, 1.361357 */
	{ 0x1915, 0x0BBB }, /* original: 3.135068, 1.466077 */
	{ 0x1970, 0x0C91 }, /* original: 3.179507, 1.570796 */
};

static const struct cherry_calib_key qm35825_partial_calib_default_keys[] = {
	/* PDOA LUT Configuration. */
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut0.data",
				     qm35825_calib_2port_pdoa_lut0_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut1.data",
				     qm35825_calib_2port_pdoa_lut1_ch9),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut2.data",
				     qm35825_calib_2port_pdoa_lut2_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut3.data",
				     qm35825_calib_2port_pdoa_lut3_ch9),
	CHERRY_CALIB_UINT8("ant_pair0.ch5.pdoa.lut_id", 0x0),
	CHERRY_CALIB_UINT8("ant_pair0.ch9.pdoa.lut_id", 0x1),
	CHERRY_CALIB_UINT8("ant_pair1.ch5.pdoa.lut_id", 0X2),
	CHERRY_CALIB_UINT8("ant_pair1.ch9.pdoa.lut_id", 0x3),
	CHERRY_CALIB_UINT8("ant_set0.tx_power_control", 0x01),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.tx_ant_paths",
				     qm35825_calib_2port_ant_set3_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set3.nb_rx_ants", 1),
	CHERRY_CALIB_BOOL("ant_set3.rx_ants_are_pairs", false),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.rx_ants",
				     qm35825_calib_2port_ant_set3_rx_ants),
	CHERRY_CALIB_UINT8("ant_set3.tx_power_control", 0x01),
};

static const struct cherry_calib_key qm35825_calib_2port_keys[] = {
	/* Global Calibration. */
	CHERRY_CALIB_UINT8("ip_sts_sanity_thres_q2", 0x0a),

	/* PDOA LUT Configuration. */
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut0.data",
				     qm35825_calib_2port_pdoa_lut0_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut1.data",
				     qm35825_calib_2port_pdoa_lut1_ch9),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut2.data",
				     qm35825_calib_2port_pdoa_lut2_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut3.data",
				     qm35825_calib_2port_pdoa_lut3_ch9),

	/* ANT0 = TX_ANT2 */
	CHERRY_CALIB_UINT32("ant0.ch5.ref_frame0.tx_power_index", 0x4a4a4a4a),
	CHERRY_CALIB_UINT32("ant0.ch9.ref_frame0.tx_power_index", 0x4a4a4a4a),
	CHERRY_CALIB_UINT32("ant0.ch5.ant_delay", 16403),
	CHERRY_CALIB_UINT32("ant0.ch9.ant_delay", 16407),
	CHERRY_CALIB_UINT8("ant0.transceiver", 0),
	CHERRY_CALIB_UINT8("ant0.port", 2),
	CHERRY_CALIB_INT8("ant0.ch5.pa_gain_offset", 0x7),
	CHERRY_CALIB_INT8("ant0.ch9.pa_gain_offset", 0x8),
	/* ANT1 = RXB_ANT2 */
	CHERRY_CALIB_UINT8("ant1.transceiver", 2),
	CHERRY_CALIB_UINT8("ant1.port", 2),
	CHERRY_CALIB_UINT8("ant1.lna", 1),
	CHERRY_CALIB_UINT32("ant1.ch5.ant_delay", 16403),
	CHERRY_CALIB_UINT32("ant1.ch9.ant_delay", 16407),

	/* ANT2 = RXA_ANT3 */
	CHERRY_CALIB_UINT8("ant2.transceiver", 1),
	CHERRY_CALIB_UINT8("ant2.port", 3),
	CHERRY_CALIB_UINT8("ant2.lna", 1),
	CHERRY_CALIB_UINT32("ant2.ch5.ant_delay", 16392),
	CHERRY_CALIB_UINT32("ant2.ch9.ant_delay", 16393),

	/* ANT3 = RXA_ANT4 */
	CHERRY_CALIB_UINT8("ant3.transceiver", 1),
	CHERRY_CALIB_UINT8("ant3.port", 4),
	CHERRY_CALIB_UINT8("ant3.lna", 1),
	CHERRY_CALIB_UINT32("ant3.ch5.ant_delay", 16396),
	CHERRY_CALIB_UINT32("ant3.ch9.ant_delay", 16395),

	/* ANT4 */
	CHERRY_CALIB_UINT8("ant4.transceiver", 0),
	CHERRY_CALIB_UINT8("ant4.port", 2),
	CHERRY_CALIB_INT8("ant4.ch5.pa_gain_offset", 0x7),
	CHERRY_CALIB_UINT32("ant4.ch5.ref_frame0.tx_power_index", 0x4a4a4a4a),
	CHERRY_CALIB_INT8("ant4.ch9.pa_gain_offset", 0x8),
	CHERRY_CALIB_UINT32("ant4.ch9.ref_frame0.tx_power_index", 0x4a4a4a4a),
	CHERRY_CALIB_UINT32("ant4.ch5.ant_delay", 16403),
	CHERRY_CALIB_UINT32("ant4.ch9.ant_delay", 16407),

	/* Ant Pair 0 to measure Azimuth between ant1 and ant3 */
	CHERRY_CALIB_UINT8("ant_pair0.axis", 0),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_pair0.ant_paths",
				     qm35825_calib_2port_ant_pair_0_ant_path),
	CHERRY_CALIB_INT16("ant_pair0.ch5.pdoa.offset", 3893),
	CHERRY_CALIB_INT16("ant_pair0.ch9.pdoa.offset", 1669),
	CHERRY_CALIB_UINT8("ant_pair0.ch5.pdoa.lut_id", 0x0),
	CHERRY_CALIB_UINT8("ant_pair0.ch9.pdoa.lut_id", 0x1),

	/* Ant Pair 1 to measure Elevation between ant1 and ant2 */
	CHERRY_CALIB_UINT8("ant_pair1.axis", 1),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_pair1.ant_paths",
				     qm35825_calib_2port_ant_pair_1_ant_path),
	CHERRY_CALIB_INT16("ant_pair1.ch5.pdoa.offset", 4476),
	CHERRY_CALIB_INT16("ant_pair1.ch9.pdoa.offset", 1716),
	CHERRY_CALIB_UINT8("ant_pair1.ch5.pdoa.lut_id", 0X2),
	CHERRY_CALIB_UINT8("ant_pair1.ch9.pdoa.lut_id", 0x3),

	/* Ant Set0: TWR ranging — ant_pair1 (ANT2+ANT3) for PDOA. */
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set0.tx_ant_paths",
				     qm35825_calib_2port_ant_set0_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set0.nb_rx_ants", 1),
	CHERRY_CALIB_BOOL("ant_set0.rx_ants_are_pairs", true),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set0.rx_ants",
				     qm35825_calib_2port_ant_set0_rx_ants),
	CHERRY_CALIB_UINT8("ant_set0.tx_power_control", 0x01),

	/* Ant Set3 : for radar. */
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.tx_ant_paths",
				     qm35825_calib_2port_ant_set3_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set3.nb_rx_ants", 1),
	CHERRY_CALIB_BOOL("ant_set3.rx_ants_are_pairs", false),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.rx_ants",
				     qm35825_calib_2port_ant_set3_rx_ants),
	CHERRY_CALIB_UINT8("ant_set3.tx_power_control", 0x01),

	/* Ant Set for Mac Mode 1. */

	/* ant4 */
	CHERRY_CALIB_UINT8("ant4.transceiver", 0x02),
	CHERRY_CALIB_UINT8("ant4.port", 0x01),
	CHERRY_CALIB_UINT8("ant4.lna", 1),

	/* ant5 */
	CHERRY_CALIB_UINT8("ant5.transceiver", 0x02),
	CHERRY_CALIB_UINT8("ant5.port", 0x02),
	CHERRY_CALIB_UINT8("ant5.lna", 1),

	/* ant6 */
	CHERRY_CALIB_UINT8("ant6.transceiver", 0x00),
	CHERRY_CALIB_UINT8("ant6.port", 0x01),
	CHERRY_CALIB_UINT8("ant6.lna", 0),

	/* ant7 */
	CHERRY_CALIB_UINT8("ant7.transceiver", 0x00),
	CHERRY_CALIB_UINT8("ant7.port", 0x02),
	CHERRY_CALIB_UINT8("ant7.lna", 0),

	/* Ant Set 4 */
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set4.tx_ant_paths",
				     qm35825_calib_2port_ant_set4_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set4.nb_rx_ants", 0x01),
	CHERRY_CALIB_BOOL("ant_set4.rx_ants_are_pairs", false),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set4.rx_ants",
				     qm35825_calib_2port_ant_set4_rx_ants),

	/* Ant Set 5 */
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set5.tx_ant_paths",
				     qm35825_calib_2port_ant_set5_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set5.nb_rx_ants", 0x01),
	CHERRY_CALIB_BOOL("ant_set5.rx_ants_are_pairs", false),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set5.rx_ants",
				     qm35825_calib_2port_ant_set5_rx_ants),
};

static const struct cherry_calib qm35825_calib_2port = {
	.n_keys = sizeof(qm35825_calib_2port_keys) /
		  sizeof(qm35825_calib_2port_keys[0]),
	.keys = qm35825_calib_2port_keys,
};

static const struct cherry_calib qm35825_partial_calib_default = {
	.n_keys = sizeof(qm35825_partial_calib_default_keys) /
		  sizeof(qm35825_partial_calib_default_keys[0]),
	.keys = qm35825_partial_calib_default_keys,
};
