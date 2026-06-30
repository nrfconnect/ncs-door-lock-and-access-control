/*
 * Copyright (c) 2026 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>

#include <cmsis_core.h>
#include <SEGGER_RTT.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/fatal.h>
#include <zephyr/sys/util.h>

static void rtt_str(const char *str)
{
	SEGGER_RTT_WriteString(0, str);
}

static void rtt_hex(const char *name, uint32_t value)
{
	char text[] = "0x00000000\n";

	for (int i = 0; i < 8; i++) {
		uint8_t nibble = (value >> ((7 - i) * 4)) & 0x0f;

		text[2 + i] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
	}

	rtt_str(name);
	rtt_str(text);
}

static void rtt_dec(const char *name, unsigned int value)
{
	char digits[11];
	int pos = ARRAY_SIZE(digits);

	digits[--pos] = '\0';
	if (value == 0) {
		digits[--pos] = '0';
	} else {
		while (value > 0 && pos > 0) {
			digits[--pos] = '0' + (value % 10);
			value /= 10;
		}
	}

	rtt_str(name);
	rtt_str(&digits[pos]);
	rtt_str("\n");
}

void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf)
{
	SEGGER_RTT_Init();
	rtt_str("\n[FATAL_RTT] Zephyr fatal handler\n");
	rtt_dec("[FATAL_RTT] reason=", reason);

	if (esf != NULL) {
		rtt_hex("[FATAL_RTT] r0  = ", esf->basic.r0);
		rtt_hex("[FATAL_RTT] r1  = ", esf->basic.r1);
		rtt_hex("[FATAL_RTT] r2  = ", esf->basic.r2);
		rtt_hex("[FATAL_RTT] r3  = ", esf->basic.r3);
		rtt_hex("[FATAL_RTT] r12 = ", esf->basic.r12);
		rtt_hex("[FATAL_RTT] lr  = ", esf->basic.lr);
		rtt_hex("[FATAL_RTT] pc  = ", esf->basic.pc);
		rtt_hex("[FATAL_RTT] xpsr= ", esf->basic.xpsr);
	} else {
		rtt_str("[FATAL_RTT] esf=NULL\n");
	}

	rtt_hex("[FATAL_RTT] CFSR= ", SCB->CFSR);
	rtt_hex("[FATAL_RTT] HFSR= ", SCB->HFSR);
	rtt_hex("[FATAL_RTT] DFSR= ", SCB->DFSR);
	rtt_hex("[FATAL_RTT] MMFAR=", SCB->MMFAR);
	rtt_hex("[FATAL_RTT] BFAR= ", SCB->BFAR);

	(void)arch_irq_lock();
	for (;;) {
	}
}
