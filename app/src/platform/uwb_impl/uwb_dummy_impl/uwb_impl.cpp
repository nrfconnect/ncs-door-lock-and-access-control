/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"

namespace Aliro::Uwb {

AliroError UltraWideBandImpl::_Init([[maybe_unused]] const Callbacks &)
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_Deinit()
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

void UltraWideBandImpl::_BleTimeSync()
	const { /* No operation for dummy implementation; override in derived classes if needed. */ };

AliroError UltraWideBandImpl::_HandleBleMessage([[maybe_unused]] const uint8_t *, [[maybe_unused]] size_t)
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_ConfigureRangingSession([[maybe_unused]] SessionIdentifier,
						       [[maybe_unused]] const CryptoTypes::Ursk &,
						       [[maybe_unused]] void *)
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_InitiateRangingSession()
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_TerminateRangingSession()
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_SuspendRangingSession()
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

AliroError UltraWideBandImpl::_ResumeRangingSession()
{
	return ALIRO_ERROR_NOT_IMPLEMENTED;
}

} // namespace Aliro::Uwb
