/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <l2cap_server/l2cap_server.h>

#include <doorlock/utils/memory.h>
#include <doorlock/utils/mutex_guard.h>
#include <doorlock/utils/utils.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(L2CAPServer, CONFIG_DOOR_LOCK_ALIRO_L2CAP_SERVER_LOG_LEVEL);

namespace DoorLock::L2capServer {

namespace {

using Utils::MutexGuard;

struct L2capChanNode {
	sys_snode_t node{};

	bt_conn *conn{ nullptr };
	bt_l2cap_le_chan chan{};
};

static_assert(offsetof(L2capChanNode, node) == 0);

Callbacks sCallbacks{};
bt_l2cap_server sServer{};
uint16_t sSpsm{};
size_t sChannelCount{ 0 };

void Connected(bt_l2cap_chan *channel);
void Disconnected(bt_l2cap_chan *channel);
int DataReceived(bt_l2cap_chan *channel, net_buf *buffer);
void Released(bt_l2cap_chan *channel);

constexpr bt_l2cap_chan_ops sChannelCallbacks{
	.connected = Connected,
	.disconnected = Disconnected,
	.recv = DataReceived,
	.released = Released,
};

static_assert(BT_L2CAP_TX_MTU >= 267, "BT_L2CAP_TX_MTU must be at least 267 bytes");
static_assert(BT_L2CAP_RX_MTU >= 264, "BT_L2CAP_RX_MTU must be at least 264 bytes");
static_assert(CONFIG_BT_MAX_CONN >= CONFIG_DOOR_LOCK_ALIRO_L2CAP_SERVER_MAX_SESSIONS,
	      "CONFIG_BT_MAX_CONN must be greater than or equal to the configured Aliro L2CAP server session limit");

NET_BUF_POOL_DEFINE(sNetBufPool, CONFIG_DOOR_LOCK_ALIRO_L2CAP_SERVER_MAX_SESSIONS,
		    BT_L2CAP_SDU_BUF_SIZE(BT_L2CAP_SDU_TX_MTU), CONFIG_BT_CONN_TX_USER_DATA_SIZE, nullptr);

K_MUTEX_DEFINE(sL2capChanMutex);
sys_slist_t sL2capChanList{};

bt_l2cap_le_chan *NewChannel(bt_conn *conn)
{
	auto *ref = bt_conn_ref(conn);
	VerifyOrReturnValue(ref, nullptr, LOG_ERR("Cannot reference connection (conn: %p)", conn));

	auto *node = DoorLock::Utils::new_nothrow<L2capChanNode>();
	if (!node) {
		LOG_ERR("Cannot allocate channel node (conn: %p)", ref);
		bt_conn_unref(ref);
		return nullptr;
	}

	node->conn = ref;

	{
		MutexGuard lock{ sL2capChanMutex };
		sys_slist_append(&sL2capChanList, &node->node);
	}

	return &node->chan;
}

int SendToChannel(bt_conn *conn, net_buf *buffer)
{
	sys_snode_t *node{ nullptr };

	{
		MutexGuard lock{ sL2capChanMutex };

		SYS_SLIST_FOR_EACH_NODE (&sL2capChanList, node) {
			auto *nodeObj = CONTAINER_OF(node, L2capChanNode, node);
			if (nodeObj->conn == conn) {
				bt_l2cap_chan *chan{ &nodeObj->chan.chan };
				return bt_l2cap_chan_send(chan, buffer);
			}
		}
	}

	return -ENOTCONN;
}

void FreeChannel(bt_l2cap_le_chan *chan)
{
	auto *node = CONTAINER_OF(chan, L2capChanNode, chan);

	{
		MutexGuard lock{ sL2capChanMutex };
		sys_slist_find_and_remove(&sL2capChanList, &node->node);
	}

	bt_conn_unref(node->conn);
	delete node;
}

bool IsValidDynamicSpsm(uint16_t spsm)
{
	constexpr uint16_t kL2capSpsmMin{ 0x0080 };
	constexpr uint16_t kL2capSpsmMax{ 0x00FF };

	return IN_RANGE(spsm, kL2capSpsmMin, kL2capSpsmMax);
}

int Accept(bt_conn *conn, bt_l2cap_server *server, bt_l2cap_chan **channel)
{
	VerifyOrReturnValue(server && channel, -EINVAL, LOG_ERR("Invalid argument"));
	VerifyOrReturnValue(sSpsm == server->psm, -ENOTSUP, LOG_ERR("Invalid PSM value"));
	VerifyOrReturnValue(sChannelCount < CONFIG_DOOR_LOCK_ALIRO_L2CAP_SERVER_MAX_SESSIONS, -ENOMEM,
			    LOG_ERR("Too many connections"));

	VerifyOrReturnValue(sCallbacks.mAccept, -EIO, LOG_ERR("Accept callback is not set"));
	VerifyOrReturnValue(sCallbacks.mAccept(conn), -EACCES,
			    LOG_ERR("Incoming connection rejected (conn: %p)", conn));

	auto *l2capChan = NewChannel(conn);
	VerifyOrReturnValue(l2capChan, -ENOMEM, LOG_ERR("Cannot allocate channel (conn: %p)", conn));

	l2capChan->chan.ops = &sChannelCallbacks;
	*channel = &l2capChan->chan;

	sChannelCount++;

	return 0;
}

void Connected(bt_l2cap_chan *channel)
{
	VerifyOrReturn(channel, LOG_ERR("Invalid argument"));

	LOG_INF("L2CAP connected: %p", channel);

	VerifyAndCall(sCallbacks.mOnConnected, channel->conn);
}

void Disconnected(bt_l2cap_chan *channel)
{
	VerifyOrReturn(channel, LOG_ERR("Invalid argument"));

	LOG_INF("L2CAP disconnected: %p", channel);

	VerifyAndCall(sCallbacks.mOnDisconnected, channel->conn);
}

int DataReceived(bt_l2cap_chan *channel, net_buf *buffer)
{
	VerifyOrReturnValue(channel && buffer, -EINVAL, LOG_ERR("Invalid argument"));

	LOG_HEXDUMP_DBG(buffer->data, buffer->len, "L2CAP received: ");

	VerifyAndCall(sCallbacks.mOnDataReceived, channel->conn, buffer->data, buffer->len);

	return 0;
}

void Released(bt_l2cap_chan *channel)
{
	VerifyOrReturn(channel, LOG_ERR("Invalid argument"));

	auto *chan = BT_L2CAP_LE_CHAN(channel);
	FreeChannel(chan);

	sChannelCount--;
}

} // namespace

int Init(const Callbacks &callbacks)
{
	sCallbacks = callbacks;

	sServer.accept = Accept;

	int error = bt_l2cap_server_register(&sServer);
	VerifyOrReturnValue(error == 0, error, LOG_ERR("Cannot register L2CAP server (error: %d)", error));

	sSpsm = sServer.psm;
	LOG_DBG("L2CAP server registered with PSM: 0x%04x", sSpsm);
	VerifyOrReturnValue(IsValidDynamicSpsm(sSpsm), -EIO, LOG_ERR("Invalid PSM value"));

	return 0;
}

uint16_t GetSpsm()
{
	return sSpsm;
}

int Send(bt_conn *conn, const uint8_t *data, size_t length)
{
	VerifyOrReturnValue(conn, -EINVAL, LOG_ERR("Invalid connection"));
	VerifyOrReturnValue(data && length, -EINVAL, LOG_ERR("Invalid data or length"));

	LOG_HEXDUMP_DBG(data, length, "L2CAP send: ");

	net_buf *buffer = net_buf_alloc(&sNetBufPool, K_FOREVER);
	VerifyOrReturnValue(buffer, -ENOMEM, LOG_ERR("Cannot allocate buffer"));

	net_buf_reserve(buffer, BT_L2CAP_SDU_CHAN_SEND_RESERVE);

	int error{ 0 };

	VerifyOrExit(net_buf_max_len(buffer) >= length, error = -ENOMEM;
		     LOG_ERR("L2CAP send failed: data too large %zu > %u", length, net_buf_max_len(buffer)));

	net_buf_add_mem(buffer, data, length);

	error = SendToChannel(conn, buffer);
	VerifyOrExit(error == 0, LOG_ERR("L2CAP send failed: %d", error););

	return 0;

exit:
	net_buf_unref(buffer);
	return error;
}

} // namespace DoorLock::L2capServer
