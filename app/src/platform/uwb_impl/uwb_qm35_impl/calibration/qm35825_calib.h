/*
 * Header file for chip calibration.
 *
 * SPDX-FileCopyrightText: Copyright (c) 2025 Qorvo, Inc.
 * SPDX-License-Identifier: LicenseRef-QORVO-2
 */

#pragma once

#include <cherry/cherry.h>

/* Modified calibration based on jolie_quad_GSavg_27032025.json. */
static const uint8_t util_calib_qm35825_ant_pair_0_ant_path[] = { 0x03, 0x01 };
static const uint8_t util_calib_qm35825_ant_pair_1_ant_path[] = { 0x02, 0x01 };
static const uint8_t util_calib_qm35825_ant_set0_tx_ant_paths[] = { 0x00,
								    0xFF };
static const uint8_t util_calib_qm35825_ant_set3_tx_ant_paths[] = { 0x00,
								    0xFF };
static const uint8_t util_calib_qm35825_ant_set0_rx_ants[] = { 0x00, 0x01,
							       0x00 };
static const uint8_t util_calib_qm35825_ant_set3_rx_ants[] = { 0x02, 0xFF,
							       0xFF };

static const uint16_t util_calib_qm35825_pdoa_lut0_ch5[][2] = {
	{ 0xEBBC, 0xF370 }, /* original: -2.533321, -1.570796 */
	{ 0xEBFB, 0xF446 }, /* original: -2.502671, -1.466077 */
	{ 0xEC95, 0xF51C }, /* original: -2.427698, -1.361357 */
	{ 0xED66, 0xF5F3 }, /* original: -2.325576, -1.256637 */
	{ 0xEE3F, 0xF6C9 }, /* original: -2.219499, -1.151917 */
	{ 0xEF0D, 0xF7A0 }, /* original: -2.118834, -1.047198 */
	{ 0xEFEF, 0xF876 }, /* original: -2.008664, -0.942478 */
	{ 0xF0F5, 0xF94D }, /* original: -1.880509, -0.837758 */
	{ 0xF22D, 0xFA23 }, /* original: -1.728161, -0.733038 */
	{ 0xF3A2, 0xFAFA }, /* original: -1.545929, -0.628319 */
	{ 0xF541, 0xFBD0 }, /* original: -1.343469, -0.523599 */
	{ 0xF701, 0xFCA7 }, /* original: -1.124806, -0.418879 */
	{ 0xF8E9, 0xFD7D }, /* original: -0.886351, -0.314159 */
	{ 0xFB09, 0xFE54 }, /* original: -0.621014, -0.209440 */
	{ 0xFD66, 0xFF2A }, /* original: -0.325595, -0.104720 */
	{ 0xFFEE, 0x0000 }, /* original: -0.008833, 0.000000 */
	{ 0x0294, 0x00D6 }, /* original: 0.322504, 0.104720 */
	{ 0x0529, 0x01AC }, /* original: 0.645128, 0.209440 */
	{ 0x0791, 0x0283 }, /* original: 0.946167, 0.314159 */
	{ 0x09BE, 0x0359 }, /* original: 1.217986, 0.418879 */
	{ 0x0B99, 0x0430 }, /* original: 1.450172, 0.523599 */
	{ 0x0D1F, 0x0506 }, /* original: 1.640173, 0.628319 */
	{ 0x0E84, 0x05DD }, /* original: 1.814894, 0.733038 */
	{ 0x100C, 0x06B3 }, /* original: 2.006023, 0.837758 */
	{ 0x11E4, 0x078A }, /* original: 2.236502, 0.942478 */
	{ 0x13E9, 0x0860 }, /* original: 2.489156, 1.047198 */
	{ 0x159C, 0x0937 }, /* original: 2.701204, 1.151917 */
	{ 0x16D1, 0x0A0D }, /* original: 2.852178, 1.256637 */
	{ 0x1798, 0x0AE4 }, /* original: 2.949266, 1.361357 */
	{ 0x181E, 0x0BBA }, /* original: 3.014801, 1.466077 */
	{ 0x18B3, 0x0C90 }, /* original: 3.087452, 1.570796 */
};

static const uint16_t util_calib_qm35825_pdoa_lut1_ch9[][2] = {
	{ 0xE276, 0xF370 }, /* original: -3.692644, -1.570796 */
	{ 0xE552, 0xF446 }, /* original: -3.334963, -1.466077 */
	{ 0xE7A9, 0xF51C }, /* original: -3.042653, -1.361357 */
	{ 0xE99B, 0xF5F3 }, /* original: -2.799788, -1.256637 */
	{ 0xEB8F, 0xF6C9 }, /* original: -2.555214, -1.151917 */
	{ 0xEDD8, 0xF7A0 }, /* original: -2.269609, -1.047198 */
	{ 0xF040, 0xF876 }, /* original: -1.968854, -0.942478 */
	{ 0xF26B, 0xF94D }, /* original: -1.698059, -0.837758 */
	{ 0xF3F7, 0xFA23 }, /* original: -1.504771, -0.733038 */
	{ 0xF4E2, 0xFAFA }, /* original: -1.389977, -0.628319 */
	{ 0xF596, 0xFBD0 }, /* original: -1.302234, -0.523599 */
	{ 0xF6D7, 0xFCA7 }, /* original: -1.145199, -0.418879 */
	{ 0xF8BD, 0xFD7D }, /* original: -0.907927, -0.314159 */
	{ 0xFB11, 0xFE54 }, /* original: -0.616846, -0.209440 */
	{ 0xFD93, 0xFF2A }, /* original: -0.303409, -0.104720 */
	{ 0xFFEC, 0x0000 }, /* original: -0.009988, 0.000000 */
	{ 0x01F6, 0x00D6 }, /* original: 0.245181, 0.104720 */
	{ 0x03A1, 0x01AC }, /* original: 0.453615, 0.209440 */
	{ 0x0518, 0x0283 }, /* original: 0.637080, 0.314159 */
	{ 0x068D, 0x0359 }, /* original: 0.819239, 0.418879 */
	{ 0x0836, 0x0430 }, /* original: 1.026568, 0.523599 */
	{ 0x0A43, 0x0506 }, /* original: 1.282985, 0.628319 */
	{ 0x0C8A, 0x05DD }, /* original: 1.567616, 0.733038 */
	{ 0x0EA8, 0x06B3 }, /* original: 1.832378, 0.837758 */
	{ 0x107F, 0x078A }, /* original: 2.062391, 0.942478 */
	{ 0x1215, 0x0860 }, /* original: 2.260298, 1.047198 */
	{ 0x137E, 0x0937 }, /* original: 2.436810, 1.151917 */
	{ 0x14D6, 0x0A0D }, /* original: 2.604729, 1.256637 */
	{ 0x1623, 0x0AE4 }, /* original: 2.767291, 1.361357 */
	{ 0x173E, 0x0BBA }, /* original: 2.905700, 1.466077 */
	{ 0x17F9, 0x0C90 }, /* original: 2.996765, 1.570796 */
};

static const uint16_t util_calib_qm35825_pdoa_lut2_ch5[][2] = {
	{ 0xE9A9, 0xF370 }, /* original: -2.792527, -1.570796 */
	{ 0xEB2C, 0xF446 }, /* original: -2.603787, -1.466077 */
	{ 0xEBA9, 0xF51C }, /* original: -2.542772, -1.361357 */
	{ 0xEBE6, 0xF5F3 }, /* original: -2.512901, -1.256637 */
	{ 0xEC73, 0xF6C9 }, /* original: -2.443996, -1.151917 */
	{ 0xED40, 0xF7A0 }, /* original: -2.343959, -1.047198 */
	{ 0xEE2F, 0xF876 }, /* original: -2.227345, -0.942478 */
	{ 0xEF79, 0xF94D }, /* original: -2.066077, -0.837758 */
	{ 0xF18A, 0xFA23 }, /* original: -1.807950, -0.733038 */
	{ 0xF3AE, 0xFAFA }, /* original: -1.540392, -0.628319 */
	{ 0xF549, 0xFBD0 }, /* original: -1.339376, -0.523599 */
	{ 0xF715, 0xFCA7 }, /* original: -1.114900, -0.418879 */
	{ 0xF8A6, 0xFD7D }, /* original: -0.919253, -0.314159 */
	{ 0xFAE7, 0xFE54 }, /* original: -0.637496, -0.209440 */
	{ 0xFDBD, 0xFF2A }, /* original: -0.282860, -0.104720 */
	{ 0xFFF8, 0x0000 }, /* original: -0.003959, 0.000000 */
	{ 0x02B5, 0x00D6 }, /* original: 0.338599, 0.104720 */
	{ 0x0598, 0x01AC }, /* original: 0.699250, 0.209440 */
	{ 0x07DF, 0x0283 }, /* original: 0.984358, 0.314159 */
	{ 0x0A49, 0x0359 }, /* original: 1.285751, 0.418879 */
	{ 0x0CA8, 0x0430 }, /* original: 1.582377, 0.523599 */
	{ 0x0E60, 0x0506 }, /* original: 1.796904, 0.628319 */
	{ 0x0FFC, 0x05DD }, /* original: 1.998268, 0.733038 */
	{ 0x1162, 0x06B3 }, /* original: 2.173067, 0.837758 */
	{ 0x1251, 0x078A }, /* original: 2.289824, 0.942478 */
	{ 0x12D9, 0x0860 }, /* original: 2.356255, 1.047198 */
	{ 0x135E, 0x0937 }, /* original: 2.421218, 1.151917 */
	{ 0x13E1, 0x0A0D }, /* original: 2.484917, 1.256637 */
	{ 0x1429, 0x0AE4 }, /* original: 2.520422, 1.361357 */
	{ 0x14CC, 0x0BBA }, /* original: 2.599798, 1.466077 */
	{ 0x1657, 0x0C90 }, /* original: 2.792527, 1.570796 */
};

static const uint16_t util_calib_qm35825_pdoa_lut3_ch9[][2] = {
	{ 0xE844, 0xF370 }, /* original: -2.967060, -1.570796 */
	{ 0xE91A, 0xF446 }, /* original: -2.862340, -1.466077 */
	{ 0xEA42, 0xF51C }, /* original: -2.717778, -1.361357 */
	{ 0xEB40, 0xF5F3 }, /* original: -2.593895, -1.256637 */
	{ 0xEBAA, 0xF6C9 }, /* original: -2.542119, -1.151917 */
	{ 0xEC4A, 0xF7A0 }, /* original: -2.464203, -1.047198 */
	{ 0xEE05, 0xF876 }, /* original: -2.247908, -0.942478 */
	{ 0xF181, 0xF94D }, /* original: -1.812106, -0.837758 */
	{ 0xF571, 0xFA23 }, /* original: -1.320167, -0.733038 */
	{ 0xF721, 0xFAFA }, /* original: -1.109353, -0.628319 */
	{ 0xF79D, 0xFBD0 }, /* original: -1.048388, -0.523599 */
	{ 0xF8BF, 0xFCA7 }, /* original: -0.906986, -0.418879 */
	{ 0xFA51, 0xFD7D }, /* original: -0.710901, -0.314159 */
	{ 0xFBCE, 0xFE54 }, /* original: -0.524584, -0.209440 */
	{ 0xFDB2, 0xFF2A }, /* original: -0.288408, -0.104720 */
	{ 0xFFD6, 0x0000 }, /* original: -0.020595, 0.000000 */
	{ 0x029D, 0x00D6 }, /* original: 0.326923, 0.104720 */
	{ 0x053A, 0x01AC }, /* original: 0.653783, 0.209440 */
	{ 0x07DA, 0x0283 }, /* original: 0.981670, 0.314159 */
	{ 0x0A61, 0x0359 }, /* original: 1.297462, 0.418879 */
	{ 0x0CAF, 0x0430 }, /* original: 1.585514, 0.523599 */
	{ 0x0F38, 0x0506 }, /* original: 1.902817, 0.628319 */
	{ 0x1176, 0x05DD }, /* original: 2.182937, 0.733038 */
	{ 0x1377, 0x06B3 }, /* original: 2.433119, 0.837758 */
	{ 0x151E, 0x078A }, /* original: 2.640119, 0.942478 */
	{ 0x1661, 0x0860 }, /* original: 2.797588, 1.047198 */
	{ 0x1768, 0x0937 }, /* original: 2.925985, 1.151917 */
	{ 0x183E, 0x0A0D }, /* original: 3.030629, 1.256637 */
	{ 0x18C9, 0x0AE4 }, /* original: 3.098309, 1.361357 */
	{ 0x1922, 0x0BBA }, /* original: 3.142011, 1.466077 */
	{ 0x196E, 0x0C90 }, /* original: 3.178911, 1.570796 */
};

static const struct cherry_calib_key util_partial_calib_qm35825_keys[] = {
	/* PDOA LUT Configuration. */
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut0.data",
				     util_calib_qm35825_pdoa_lut0_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut1.data",
				     util_calib_qm35825_pdoa_lut1_ch9),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut2.data",
				     util_calib_qm35825_pdoa_lut2_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut3.data",
				     util_calib_qm35825_pdoa_lut3_ch9),
	CHERRY_CALIB_UINT8("ant_pair0.ch5.pdoa.lut_id", 0x0),
	CHERRY_CALIB_UINT8("ant_pair0.ch9.pdoa.lut_id", 0x1),
	CHERRY_CALIB_UINT8("ant_pair1.ch5.pdoa.lut_id", 0X2),
	CHERRY_CALIB_UINT8("ant_pair1.ch9.pdoa.lut_id", 0x3),
	CHERRY_CALIB_UINT8("ant_set0.tx_power_control", 0x01),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.tx_ant_paths",
				     util_calib_qm35825_ant_set3_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set3.nb_rx_ants", 1),
	CHERRY_CALIB_BOOL("ant_set3.rx_ants_are_pairs", false),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.rx_ants",
				     util_calib_qm35825_ant_set3_rx_ants),
	CHERRY_CALIB_UINT8("ant_set3.tx_power_control", 0x01),
};

static const struct cherry_calib_key util_calib_qm35825_keys[] = {
	/* Global Calibration. */
	CHERRY_CALIB_UINT8("xtal_trim", 0x32),

	/* PDOA LUT Configuration. */
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut0.data",
				     util_calib_qm35825_pdoa_lut0_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut1.data",
				     util_calib_qm35825_pdoa_lut1_ch9),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut2.data",
				     util_calib_qm35825_pdoa_lut2_ch5),
	CHERRY_CALIB_NUMBER_ARRAY_2D("pdoa_lut3.data",
				     util_calib_qm35825_pdoa_lut3_ch9),

	/* ANT0 = TX_ANT1 */
	CHERRY_CALIB_UINT32("ant0.ch5.ref_frame0.tx_power_index", 0X51515151),
	CHERRY_CALIB_UINT32("ant0.ch9.ref_frame0.tx_power_index", 0x4d4d4d4d),
	CHERRY_CALIB_UINT32("ant0.ch5.ant_delay", 16387),
	CHERRY_CALIB_UINT32("ant0.ch9.ant_delay", 16394),
	CHERRY_CALIB_UINT8("ant0.transceiver", 0),
	CHERRY_CALIB_UINT8("ant0.port", 1),
	CHERRY_CALIB_INT8("ant0.ch5.pa_gain_offset", 0x5),
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
				     util_calib_qm35825_ant_pair_0_ant_path),
	CHERRY_CALIB_INT16("ant_pair0.ch5.pdoa.offset", 3893),
	CHERRY_CALIB_INT16("ant_pair0.ch9.pdoa.offset", 1669),
	CHERRY_CALIB_UINT8("ant_pair0.ch5.pdoa.lut_id", 0x0),
	CHERRY_CALIB_UINT8("ant_pair0.ch9.pdoa.lut_id", 0x1),

	/* Ant Pair 1 to measure Elevation between ant1 and ant2 */
	CHERRY_CALIB_UINT8("ant_pair1.axis", 1),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_pair1.ant_paths",
				     util_calib_qm35825_ant_pair_1_ant_path),
	CHERRY_CALIB_INT16("ant_pair1.ch5.pdoa.offset", 4476),
	CHERRY_CALIB_INT16("ant_pair1.ch9.pdoa.offset", 1716),
	CHERRY_CALIB_UINT8("ant_pair1.ch5.pdoa.lut_id", 0X2),
	CHERRY_CALIB_UINT8("ant_pair1.ch9.pdoa.lut_id", 0x3),

	/* Ant Set0: all excepted radar. */
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set0.tx_ant_paths",
				     util_calib_qm35825_ant_set0_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set0.nb_rx_ants", 2),
	CHERRY_CALIB_BOOL("ant_set0.rx_ants_are_pairs", true),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set0.rx_ants",
				     util_calib_qm35825_ant_set0_rx_ants),
	CHERRY_CALIB_UINT8("ant_set0.tx_power_control", 0x01),

	/* Ant Set3 : for radar. */
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.tx_ant_paths",
				     util_calib_qm35825_ant_set3_tx_ant_paths),
	CHERRY_CALIB_UINT8("ant_set3.nb_rx_ants", 1),
	CHERRY_CALIB_BOOL("ant_set3.rx_ants_are_pairs", false),
	CHERRY_CALIB_NUMBER_ARRAY_1D("ant_set3.rx_ants",
				     util_calib_qm35825_ant_set3_rx_ants),
	CHERRY_CALIB_UINT8("ant_set3.tx_power_control", 0x01),
        CHERRY_CALIB_BOOL("legacy_android_ccc", 0),
};

static const struct cherry_calib util_calib_qm35825 = {
	.n_keys = sizeof(util_calib_qm35825_keys) /
		  sizeof(util_calib_qm35825_keys[0]),
	.keys = util_calib_qm35825_keys,
};

static const struct cherry_calib util_partial_calib_qm35825 = {
	.n_keys = sizeof(util_partial_calib_qm35825_keys) /
		  sizeof(util_partial_calib_qm35825_keys[0]),
	.keys = util_partial_calib_qm35825_keys,
};
