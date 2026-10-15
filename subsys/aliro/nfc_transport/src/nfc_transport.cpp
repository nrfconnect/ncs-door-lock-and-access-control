/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <nfc_transport/nfc_transport.h>
#include <nfc_transport/nfc_transport_backend.h>

#include <aliro/aliro.h>

namespace Aliro::NfcTransport {

namespace {

constexpr Backend::Callbacks kCallbacks{
	.mOnDeviceDetected = [] { return AliroStack::Instance().CreateSession(ConnectionHandle::Nfc()); },
	.mOnDataReceived = [](Data data) { AliroStack::Instance().HandleSessionData(ConnectionHandle::Nfc(), data); },
	.mOnDeviceLost = [] { AliroStack::Instance().DestroySession(ConnectionHandle::Nfc()); },
};

} // namespace

AliroError Init()
{
	return Backend::Init(kCallbacks);
}

AliroError Start()
{
	return Backend::Start();
}

AliroError Stop()
{
	return Backend::Stop();
}

AliroError Send(Data data)
{
	return Backend::Send(data);
}

AliroError Terminate()
{
	return Backend::Terminate();
}

} // namespace Aliro::NfcTransport
