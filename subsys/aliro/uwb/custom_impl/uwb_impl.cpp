/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "uwb_impl.h"

#include <errno.h>

namespace Aliro::Uwb {

int UltraWideBandImpl::_Init([[maybe_unused]] const Callbacks &)
{
	return -ENOSYS;
}

int UltraWideBandImpl::_Deinit()
{
	return -ENOSYS;
}

void UltraWideBandImpl::
	_BleTimeSync() { /* No operation for dummy implementation; override in derived classes if needed. */ };

int UltraWideBandImpl::_HandleBleMessage([[maybe_unused]] const uint8_t *, [[maybe_unused]] size_t,
					 [[maybe_unused]] SessionContextHandle)
{
	return -ENOSYS;
}

int UltraWideBandImpl::_ConfigureRangingSession([[maybe_unused]] SessionIdentifier,
						[[maybe_unused]] const CryptoTypes::Ursk &,
						[[maybe_unused]] ProtocolVersion, [[maybe_unused]] SessionContextHandle)
{
	return -ENOSYS;
}

int UltraWideBandImpl::_InitiateRangingSession([[maybe_unused]] SessionContextHandle)
{
	return -ENOSYS;
}

int UltraWideBandImpl::_TerminateRangingSession([[maybe_unused]] SessionContextHandle)
{
	return -ENOSYS;
}

int UltraWideBandImpl::_SuspendRangingSession([[maybe_unused]] SessionContextHandle)
{
	return -ENOSYS;
}

int UltraWideBandImpl::_ResumeRangingSession([[maybe_unused]] SessionContextHandle)
{
	return -ENOSYS;
}

} // namespace Aliro::Uwb
