/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "aliro/aliro.h"
#include "aliro/shell.h"
#include "aliro/utils.h"

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <cstdio>

LOG_MODULE_REGISTER(door_lock_app, CONFIG_NCS_DOOR_LOCK_APP_LOG_LEVEL);

constexpr uint8_t kAccessAsciiArtString[] = { " █████╗  ██████╗ ██████╗███████╗███████╗███████╗ \r\n"
					      "██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝██╔════╝\r\n"
					      "███████║██║     ██║     █████╗  ███████╗███████╗\r\n"
					      "██╔══██║██║     ██║     ██╔══╝  ╚════██║╚════██║\r\n"
					      "██║  ██║╚██████╗╚██████╗███████╗███████║███████║\r\n"
					      "╚═╝  ╚═╝ ╚═════╝ ╚═════╝╚══════╝╚══════╝╚══════╝\r\n" };

constexpr uint8_t kDeniedAsciiArtString[] = { "██████╗ ███████╗███╗   ██╗██╗███████╗██████╗ \r\n"
					      "██╔══██╗██╔════╝████╗  ██║██║██╔════╝██╔══██╗\r\n"
					      "██║  ██║█████╗  ██╔██╗ ██║██║█████╗  ██║  ██║\r\n"
					      "██║  ██║██╔══╝  ██║╚██╗██║██║██╔══╝  ██║  ██║\r\n"
					      "██████╔╝███████╗██║ ╚████║██║███████╗██████╔╝\r\n"
					      "╚═════╝ ╚══════╝╚═╝  ╚═══╝╚═╝╚══════╝╚═════╝ \r\n" };

constexpr uint8_t kGrantedAsciiArtString[] = { " ██████╗ ██████╗  █████╗ ███╗   ██╗████████╗███████╗██████╗ \r\n"
					       "██╔════╝ ██╔══██╗██╔══██╗████╗  ██║╚══██╔══╝██╔════╝██╔══██╗\r\n"
					       "██║  ███╗██████╔╝███████║██╔██╗ ██║   ██║   █████╗  ██║  ██║\r\n"
					       "██║   ██║██╔══██╗██╔══██║██║╚██╗██║   ██║   ██╔══╝  ██║  ██║\r\n"
					       "╚██████╔╝██║  ██║██║  ██║██║ ╚████║   ██║   ███████╗██████╔╝\r\n"
					       " ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═════╝ \r\n" };

int main()
{
	LOG_INF("                                                                        \r\n"
		"                                          @@@   @@@@                    \r\n"
		"    @@@@@@@@@@@@                          @@@                           \r\n"
		"  @@@@@      @@@@@              @@@@   @@ @@@    @@      @@     @@@@    \r\n"
		" @@@@          @@@@          @@@@  @@@@@@ @@@    @@   @@@@@  @@@   @@@@ \r\n"
		"@@@@     @@      @@@        @@@       @@@ @@@    @@  @@@   @@@       @@@\r\n"
		"@@@     @@@@@    @@@       @@@        @@@ @@@    @@  @@    @@@        @@\r\n"
		"@@@   @@@@@@@@   @@@       @@@        @@@ @@@    @@  @@    @@@        @@\r\n"
		" @@@    @@@@    @@@@        @@@      @@@@ @@@    @@  @@     @@       @@@\r\n"
		"  @@@   @@@@   @@@@           @@@@@@@@@@@  @@@@@ @@  @@      @@@@@@@@@  \r\n"
		"   @@@@ @@@@ @@@@                                                       \r\n"
		"     @@ @@@@ @@                                                         \r\n");

	AliroError ec = Aliro::AliroStack::Instance().Init(
		{ .mOnAccessAttempt =
			  [](Aliro::Access::Status status) {
				  printf("\n%s", kAccessAsciiArtString);
				  if (status == Aliro::Access::Status::Denied) {
					  printf("\n%s", kGrantedAsciiArtString);
				  } else {
					  printf("\n%s", kDeniedAsciiArtString);
				  }
				  printf("\n");
			  },
		  .mOnError = [](AliroError error) { LOG_ERR("Aliro error: %s", error.ToString()); } });

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack initialization failed");

	ec = Aliro::AliroStack::Instance().Start();

	VerifyOrDie(ec == ALIRO_NO_ERROR, "Aliro stack start failed");

	Aliro::RegisterShellCommands();

	return 0;
}
