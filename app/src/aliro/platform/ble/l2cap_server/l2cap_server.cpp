/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "l2cap_server.h"

#include "aliro/aliro.h"
#include "aliro/memory.h"
#include "aliro/mutex_guard.h"
#include "aliro/utils.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(L2CAPServer, CONFIG_NCS_ALIRO_BLE_LOG_LEVEL);

namespace {

struct L2CapChanNode {
	sys_snode_t node{};

	bt_conn *conn{ nullptr };
	bt_l2cap_le_chan chan{};
};

static_assert(offsetof(L2CapChanNode, node) == 0);

NET_BUF_POOL_DEFINE(sNetBufPool, CONFIG_ALIRO_BLE_UWB_MAX_SESSIONS, BT_L2CAP_SDU_BUF_SIZE(BT_L2CAP_SDU_TX_MTU),
		    CONFIG_BT_CONN_TX_USER_DATA_SIZE, nullptr);

K_MUTEX_DEFINE(sL2CapChanMutex);
sys_slist_t sL2CapChanList{};

bt_l2cap_le_chan *AllocateL2capChan(bt_conn *conn)
{
	auto *ref = bt_conn_ref(conn);
	if (!ref) {
		return nullptr;
	}

	auto node = Aliro::new_nothrow<L2CapChanNode>();
	if (!node) {
		return nullptr;
	}

	node->conn = ref;

	{
		MutexGuard lock{ sL2CapChanMutex };
		sys_slist_append(&sL2CapChanList, &node->node);
	}

	return &node->chan;
}

bt_l2cap_le_chan *GetL2capChan(bt_conn *conn)
{
	bt_l2cap_le_chan *chan{ nullptr };
	sys_snode_t *node{ nullptr };

	{
		MutexGuard lock{ sL2CapChanMutex };

		SYS_SLIST_FOR_EACH_NODE (&sL2CapChanList, node) {
			auto *nodeObj = CONTAINER_OF(node, L2CapChanNode, node);
			if (nodeObj->conn == conn) {
				chan = &nodeObj->chan;
				break;
			}
		}
	}

	return chan;
}

void FreeL2capChan(bt_l2cap_le_chan *chan)
{
	auto *node = CONTAINER_OF(chan, L2CapChanNode, chan);

	{
		MutexGuard lock{ sL2CapChanMutex };
		sys_slist_find_and_remove(&sL2CapChanList, &node->node);
	}

	bt_conn_unref(node->conn);
	delete node;
}

} // namespace

namespace Aliro {

int L2capServer::Accept(bt_conn *conn, bt_l2cap_server *server, bt_l2cap_chan **channel)
{
	VerifyOrReturnValue(server && channel, -EINVAL, LOG_ERR("Invalid argument"));
	VerifyOrReturnValue(Instance().mSpsm == server->psm, -ENOENT, LOG_ERR("Invalid PSM value"));
	VerifyOrReturnValue(Instance().mChannelCount < CONFIG_ALIRO_BLE_UWB_MAX_SESSIONS, -ENOMEM,
			    LOG_ERR("Too many connections"));

	auto newChannel = AllocateL2capChan(conn);
	VerifyOrReturnValue(newChannel, -ENOMEM, LOG_ERR("Cannot allocate channel (conn: %p)", conn));

	newChannel->chan.ops = &Instance().mChannelCallbacks;

	*channel = &newChannel->chan;

	Instance().mChannelCount++;

	return 0;
}

void L2capServer::Connected(bt_l2cap_chan *channel)
{
	VerifyOrReturn(channel, LOG_ERR("Invalid argument"));

	LOG_INF("L2CAP connected: %p", channel);

	VerifyAndCall(Instance().mCallbacks.mOnConnected, channel->conn);
}

void L2capServer::Disconnected(bt_l2cap_chan *channel)
{
	VerifyOrReturn(channel, LOG_ERR("Invalid argument"));

	LOG_INF("L2CAP disconnected: %p", channel);

	VerifyAndCall(Instance().mCallbacks.mOnDisconnected, channel->conn);
}

int L2capServer::DataReceived(bt_l2cap_chan *channel, net_buf *buffer)
{
	VerifyOrReturnValue(channel && buffer, -EINVAL, LOG_ERR("Invalid argument"));

	LOG_HEXDUMP_DBG(buffer->data, buffer->len, "L2CAP received: ");

	VerifyAndCall(Instance().mCallbacks.mOnDataReceived, channel->conn, buffer->data, buffer->len);

	return 0;
}

void L2capServer::Released(bt_l2cap_chan *channel)
{
	auto chan = BT_L2CAP_LE_CHAN(channel);
	FreeL2capChan(chan);

	Instance().mChannelCount--;
}

AliroError L2capServer::Init()
{
	mChannelCallbacks.connected = L2capServer::Instance().Connected;
	mChannelCallbacks.disconnected = L2capServer::Instance().Disconnected;
	mChannelCallbacks.recv = L2capServer::Instance().DataReceived;
	mChannelCallbacks.released = L2capServer::Instance().Released;

	mL2capServer.accept = L2capServer::Accept;

	int error = bt_l2cap_server_register(&mL2capServer);
	VerifyOrReturnStatus(error == 0, ALIRO_ERROR_INTERNAL,
			     LOG_ERR("Cannot register L2CAP server (error: %d)", error));

	mSpsm = mL2capServer.psm;
	LOG_DBG("L2CAP server registered with PSM: 0x%04x", mSpsm);
	VerifyOrReturnStatus(IsValidDynamicSpsm(mSpsm), ALIRO_ERROR_INTERNAL, LOG_ERR("Invalid PSM value"));

	return ALIRO_NO_ERROR;
}

bool L2capServer::IsValidDynamicSpsm(Spsm spsm)
{
	return IN_RANGE(spsm, kL2capSpsmMin, kL2capSpsmMax);
}

L2capServer::Spsm L2capServer::GetSpsm() const
{
	return mSpsm;
}

AliroError L2capServer::Send(bt_conn *conn, const uint8_t *data, size_t length) const
{
	VerifyOrReturnStatus(data && length, ALIRO_INVALID_ARGUMENT, LOG_ERR("Invalid data or length"));

	LOG_HEXDUMP_DBG(data, length, "L2CAP send: ");

	auto chan = GetL2capChan(conn);
	VerifyOrReturnStatus(chan, ALIRO_INVALID_STATE, LOG_ERR("Cannot get channel (conn: %p)", conn));

	net_buf *buffer = net_buf_alloc(&sNetBufPool, K_FOREVER);
	VerifyOrReturnStatus(buffer, ALIRO_NO_MEMORY, LOG_ERR("Cannot allocate buffer"));

	net_buf_reserve(buffer, BT_L2CAP_SDU_CHAN_SEND_RESERVE);

	VerifyOrReturnStatus(net_buf_max_len(buffer) >= length, ALIRO_NO_MEMORY,
			     LOG_ERR("L2CAP send failed: data too large %d > %d", length, net_buf_max_len(buffer)));

	net_buf_add_mem(buffer, data, length);

	int error = bt_l2cap_chan_send(&chan->chan, buffer);
	if (error != 0) {
		LOG_ERR("L2CAP send failed: %d", error);

		net_buf_unref(buffer);

		switch (error) {
		case -ENOTCONN:
			[[fallthrough]];
		case -ESHUTDOWN:
			return ALIRO_INVALID_STATE;
		case -EMSGSIZE:
			[[fallthrough]];
		case -EINVAL:
			return ALIRO_INVALID_ARGUMENT;
		default:
			return ALIRO_ERROR_INTERNAL;
		}
	}

	return ALIRO_NO_ERROR;
}

} // namespace Aliro
