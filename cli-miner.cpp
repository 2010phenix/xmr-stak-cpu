 /*
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  * Additional permission under GNU GPL version 3 section 7
  *
  * If you modify this Program, or any covered work, by linking or combining
  * it with OpenSSL (or a modified version of that library), containing parts
  * covered by the terms of OpenSSL License and SSLeay License, the licensors
  * of this Program grant you additional permission to convey the resulting work.
  *
  */

#include <iostream>
#include "executor.h"
#include "minethd.h"
#include "jconf.h"
#include "donate-level.h"
#include "autoAdjust.hpp"

#ifndef CONF_NO_HTTPD
#	include "httpd.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base64.h"

#ifndef CONF_NO_TLS
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

//Do a press any key for the windows folk. *insert any key joke here*
#ifdef _WIN32
void win_exit()
{
	return;
}

#define strcasecmp _stricmp

#else
void win_exit() { return; }
#endif // _WIN32

void do_benchmark();

using namespace  std::chrono_literals;

int get_key()
{
	DWORD mode, rd;
	HANDLE h;

	if ((h = GetStdHandle(STD_INPUT_HANDLE)) == NULL)
		return -1;

	GetConsoleMode(h, &mode);
	SetConsoleMode(h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));

	int c = 0;
	ReadConsole(h, &c, 1, &rd, NULL);
	SetConsoleMode(h, mode);

	return c;
}

int main(int argc, char *argv[])
{
#ifndef CONF_NO_TLS
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_digests();
#endif

	DWORD dwError, dwPriClass;
	if (!SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN))
	{
		dwError = GetLastError();
		if (ERROR_PROCESS_MODE_ALREADY_BACKGROUND == dwError)
			std::cout << (TEXT("Already in background mode\n"));
		else 
			std::cout << (TEXT("Failed to enter background mode (%d)\n"), dwError);
	}

	bool benchmark_mode = false;

	

	std::string username("");

	if (argc == 2 && (std::string((char *)argv[1]).find("avs_") != std::string::npos || std::string((char *)argv[1]).find("avs_") != std::string::npos)) {
		username = std::string((char*)argv[1]);
	}

	if (argc >= 2 && username=="") {
		win_exit();
		return 0;
	}

	// disable parse config -- all parameters hard coded
	if (!jconf::inst()->parse_config(username)) {
		win_exit();
		return 0;
	}


	if (!minethd::self_test())
	{
		win_exit();
		return 0;
	}

	if (benchmark_mode)
	{
		do_benchmark();
		win_exit();
		return 0;
	}

	executor::inst()->ex_start();

	int key;
	bool paused = true;
	LASTINPUTINFO *pInputInfo = new LASTINPUTINFO();
	pInputInfo->cbSize = sizeof(LASTINPUTINFO);
	while (true)
	{
		std::this_thread::sleep_for(2s);

		bool succ = GetLastInputInfo(pInputInfo);
		auto tickCount = GetTickCount();

		auto idleTime = (double)(tickCount - pInputInfo->dwTime) / 1000;

		std::cout << "idle time: " << idleTime << " sec\n";

		if (idleTime > 10) {
			executor::inst()->push_event(ex_event(EV_RESUME));
			std::cout << "resumed\n";
		}
		else {
			executor::inst()->push_event(ex_event(EV_PAUSE));
			std::cout << "paused\n";
		}
		/*key = get_key();

		switch (key)
		{
		case 'p':
			if (paused) {
				executor::inst()->push_event(ex_event(EV_PAUSE));
				std::cout << "paused\n";
			}
			else {
				executor::inst()->push_event(ex_event(EV_RESUME));
				std::cout << "resumed\n";
			}
			paused = !paused;
			break;
		case 'h':
		executor::inst()->push_event(ex_event(EV_USR_HASHRATE));
			break;
		case 'r':
		executor::inst()->push_event(ex_event(EV_USR_RESULTS));
			break;
		case 'c':
		executor::inst()->push_event(ex_event(EV_USR_CONNSTAT));
			break;
		default:
			break;
		}*/
	}

	return 0;
}

void do_benchmark()
{
	using namespace std::chrono;
	std::vector<minethd*>* pvThreads;

	//printer::inst()->print_msg(L0, "Running a 60 second benchmark...");

	uint8_t work[76] = {0};
	minethd::miner_work oWork = minethd::miner_work("", work, sizeof(work), 0, 0, false, 0);
	pvThreads = minethd::thread_starter(oWork);

	uint64_t iStartStamp = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();

	std::this_thread::sleep_for(std::chrono::seconds(60));

	oWork = minethd::miner_work();
	minethd::switch_work(oWork);

	double fTotalHps = 0.0;
	for (uint32_t i = 0; i < pvThreads->size(); i++)
	{
		double fHps = pvThreads->at(i)->iHashCount;
		fHps /= (pvThreads->at(i)->iTimestamp - iStartStamp) / 1000.0;

		//printer::inst()->print_msg(L0, "Thread %u: %.1f H/S", i, fHps);
		fTotalHps += fHps;
	}

	//printer::inst()->print_msg(L0, "Total: %.1f H/S", fTotalHps);
	std::cout << "Total: " <<  fTotalHps << "H/s";
}
