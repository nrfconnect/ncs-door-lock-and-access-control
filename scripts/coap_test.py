#!/usr/bin/env python3
#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
"""Minimal, dependency-free CoAP tester for the door lock CoAP service.

Examples:
    scripts/coap_test.py 192.168.1.50 get
    scripts/coap_test.py 192.168.1.50 unlock
    scripts/coap_test.py 192.168.1.50 lock
    scripts/coap_test.py 192.168.1.50 observe --count 5
    scripts/coap_test.py 192.168.1.50 test      # automated end-to-end check

Use --port and --path when CONFIG_DOOR_LOCK_COAP_SERVICE_PORT or
CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH differ from the defaults.
"""

import argparse
import os
import socket
import sys
import time

GET, PUT = 1, 3
CON, NON, ACK = 0, 1, 2
OPT_OBSERVE, OPT_URI_PATH, OPT_CONTENT_FORMAT = 6, 11, 12
CF_CBOR = 60
CBOR_TRUE, CBOR_FALSE = b"\xf5", b"\xf4"
STATES = (b"secured", b"unsecured", b"blocked", b"enteringsecured", b"enteringunsecured", b"unknown")
CODES = {
    0x45: "2.05 Content",
    0x44: "2.04 Changed",
    0x80: "4.00 Bad Request",
    0x84: "4.04 Not Found",
    0x85: "4.05 Method Not Allowed",
    0x8F: "4.15 Unsupported Content-Format",
    0xA0: "5.00 Internal Server Error",
}


def encode_uint(value):
    return value.to_bytes((value.bit_length() + 7) // 8, "big") if value else b""


def build(code, mid, token, path, observe=None, payload=b"", cbor=False, msg_type=CON):
    """Build a CoAP message. msg_type: 0=CON, 1=NON, 2=ACK, 3=RST."""
    msg = bytearray([0x40 | (msg_type << 4) | len(token), code])
    msg += mid.to_bytes(2, "big") + token
    opts = []
    if observe is not None:
        opts.append((OPT_OBSERVE, encode_uint(observe)))
    opts += [(OPT_URI_PATH, seg.encode()) for seg in path.strip("/").split("/")]
    if cbor:
        opts.append((OPT_CONTENT_FORMAT, encode_uint(CF_CBOR)))
    prev = 0
    for num, val in sorted(opts, key=lambda o: o[0]):
        delta, length = num - prev, len(val)
        assert delta < 13 and length < 13, "option too large for this tiny encoder"
        msg += bytes([(delta << 4) | length]) + val
        prev = num
    if payload:
        msg += b"\xff" + payload
    return bytes(msg)


def parse(data):
    ver_type_tkl, code = data[0], data[1]
    tkl = ver_type_tkl & 0x0F
    msg = {
        "type": (ver_type_tkl >> 4) & 3,
        "code": code,
        "mid": int.from_bytes(data[2:4], "big"),
        "token": data[4:4 + tkl],
        "observe": None,
    }
    pos, num = 4 + tkl, 0
    while pos < len(data) and data[pos] != 0xFF:
        delta, length = data[pos] >> 4, data[pos] & 0x0F
        pos += 1
        for name in ("delta", "length"):
            val = delta if name == "delta" else length
            if val == 13:
                val = data[pos] + 13
                pos += 1
            elif val == 14:
                val = int.from_bytes(data[pos:pos + 2], "big") + 269
                pos += 2
            if name == "delta":
                delta = val
            else:
                length = val
        num += delta
        if num == OPT_OBSERVE:
            msg["observe"] = int.from_bytes(data[pos:pos + length], "big")
        pos += length
    msg["payload"] = data[pos + 1:] if pos < len(data) else b""
    return msg


class Client:
    def __init__(self, host, port, path, timeout):
        self.path, self.timeout = path, timeout
        self.mid = int.from_bytes(os.urandom(2), "big")
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.connect((host, port))

    def _next(self):
        self.mid = (self.mid + 1) & 0xFFFF
        return self.mid, os.urandom(4)

    def request(self, code, payload=b"", cbor=False, observe=None, msg_type=CON, token=None):
        mid, new_token = self._next()
        token = token or new_token
        self.sock.sendall(build(code, mid, token, self.path, observe, payload, cbor, msg_type))
        self.sock.settimeout(self.timeout)
        while True:
            msg = parse(self.sock.recv(1500))
            if msg["token"] != token:
                continue
            if msg["type"] == CON:  # Notification racing the response: acknowledge it
                self._ack(msg)
                continue
            return msg, token

    def _ack(self, msg):
        self.sock.sendall(bytes([0x40 | (ACK << 4), 0x00]) + msg["mid"].to_bytes(2, "big"))

    def get(self):
        return self.request(GET)[0]

    def set_locked(self, locked, msg_type=CON):
        return self.request(PUT, CBOR_FALSE if locked else CBOR_TRUE, cbor=True, msg_type=msg_type)[0]

    def wait_for_state(self, state, timeout):
        """Poll GET until the payload equals state, return the last response."""
        end = time.time() + timeout
        while True:
            r = self.get()
            if r["payload"] == state or time.time() >= end:
                return r
            time.sleep(0.2)

    def observe(self, count, duration=None):
        """Register and yield notifications (ACKing CON ones) until count reached.

        Deregisters the observation when the generator is closed.
        """
        msg, token = self.request(GET, observe=0)
        try:
            yield msg
            yield from self._notifications(token, count, duration)
        finally:
            try:
                self.request(GET, observe=1, token=token)
            except OSError:
                pass

    def _notifications(self, token, count, duration):
        end = time.time() + duration if duration else None
        seen = 1
        while seen < count:
            if end:
                self.sock.settimeout(max(0.1, end - time.time()))
            else:
                self.sock.settimeout(None)
            try:
                note = parse(self.sock.recv(1500))
            except socket.timeout:
                return
            if note["token"] != token:
                continue
            if note["type"] == CON:
                self._ack(note)
            seen += 1
            yield note

    def close(self):
        self.sock.close()


def fmt(msg):
    code = CODES.get(msg["code"], f"{msg['code'] >> 5}.{msg['code'] & 31:02d}")
    obs = f" observe={msg['observe']}" if msg["observe"] is not None else ""
    body = msg["payload"].decode(errors="replace")
    return f"{code}{obs}" + (f" payload={body!r}" if body else "")


def run_selftest(c, settle):
    failures = 0

    def check(name, cond, info=""):
        nonlocal failures
        print(f"[{'PASS' if cond else 'FAIL'}] {name} {info}")
        failures += not cond

    r = c.get()
    check("GET returns 2.05", r["code"] == 0x45, fmt(r))
    check("GET payload is a known state", r["payload"] in STATES, fmt(r))

    # The lock may move through "entering..." states before it settles.
    for locked, text in ((False, b"unsecured"), (True, b"secured")):
        r = c.set_locked(locked)
        check(f"PUT {'lock' if locked else 'unlock'} returns 2.04", r["code"] == 0x44, fmt(r))
        r = c.wait_for_state(text, settle)
        check(f"state is {text.decode()}", r["payload"] == text, fmt(r))

    r = c.request(GET, msg_type=NON)[0]
    check("NON GET returns NON 2.05", r["type"] == NON and r["code"] == 0x45, fmt(r))
    r = c.set_locked(True, msg_type=NON)
    check("NON PUT returns NON 2.04", r["type"] == NON and r["code"] == 0x44, fmt(r))

    r = c.request(PUT, b"\x01", cbor=True)[0]
    check("PUT non-boolean CBOR rejected", r["code"] == 0x80, fmt(r))
    r = c.request(PUT, CBOR_TRUE, cbor=False)[0]
    check("PUT without content-format rejected", r["code"] == 0x80, fmt(r))
    r = c.request(PUT, b"\x01", cbor=True, msg_type=NON)[0]
    check("NON PUT non-boolean CBOR rejected", r["type"] == NON and r["code"] == 0x80, fmt(r))

    # Observe: registration, then a state change must notify us.
    other = Client(*c.sock.getpeername(), c.path, c.timeout)
    notes = c.observe(count=2, duration=c.timeout * 2)
    try:
        first = next(notes)
        check("Observe registration accepted", first["observe"] is not None, fmt(first))
        other.set_locked(first["payload"] not in (b"secured", b"enteringsecured"))
        note = next(notes, None)
        check("Observer notified on change", note is not None and note["payload"] != first["payload"],
              fmt(note) if note else "(no notification)")
    finally:
        notes.close()
        other.set_locked(True)
        other.close()

    print("\nALL PASSED" if not failures else f"\n{failures} CHECK(S) FAILED")
    return failures


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("host", help="device IPv4 address")
    p.add_argument("action", choices=["get", "lock", "unlock", "observe", "test"])
    p.add_argument("--port", type=int, default=5683)
    p.add_argument("--path", default="door-lock", help="resource path (CONFIG_DOOR_LOCK_COAP_RESOURCE_PATH)")
    p.add_argument("--timeout", type=float, default=3.0)
    p.add_argument("--count", type=int, default=0, help="observe: stop after N messages (0 = until Ctrl+C)")
    p.add_argument("--settle", type=float, default=5.0,
                   help="test: max seconds to wait for a lock/unlock to complete "
                        "(CONFIG_DOOR_LOCK_ALIRO_LOCK_SIM_MOVEMENT_TIME_MS)")
    args = p.parse_args()

    c = Client(args.host, args.port, args.path, args.timeout)
    try:
        if args.action == "get":
            print(fmt(c.get()))
        elif args.action in ("lock", "unlock"):
            print(fmt(c.set_locked(args.action == "lock")))
        elif args.action == "observe":
            print("Observing, Ctrl+C to stop...")
            notes = c.observe(args.count or 1 << 30)
            try:
                for note in notes:
                    print(time.strftime("%H:%M:%S"), fmt(note))
            finally:
                notes.close()
        else:
            return 1 if run_selftest(c, args.settle) else 0
    except socket.timeout:
        print("Timeout: no response from device", file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        pass
    finally:
        c.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
