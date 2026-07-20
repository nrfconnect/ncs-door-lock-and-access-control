/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

namespace Aliro::Uwb::Dfu {

/**
 * @brief Check if DFU can be performed.
 *
 * This function compares a version stored in the QM35 FW's primary slot with
 * a QM35's version string.
 *
 * @param version QM35's version string (e.g. X.X.XrcN_BUILDID, X.X.X-rcN_BUILDID, or X.X.X_BUILDID).
 */
bool ShouldUpdate(const char *version);

/**
 * @brief Perform firmware update.
 *
 * Read new firmware from the primary slot and write it to QM35 using chunk API.
 * Note: QM35 must be restarted beforehand.
 */
int PerformFirmwareUpdate();

} // namespace Aliro::Uwb::Dfu
