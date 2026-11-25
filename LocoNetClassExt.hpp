/*
"Commons Clause" License Condition v1.0

The Software is provided to you by the Licensor under the License, as defined below, subject to the following condition.

Without limiting other conditions in the License, the grant of rights under the License will not include, and the License does not grant to you, the right to Sell the Software.

For purposes of the foregoing, "Sell" means practicing any or all of the rights granted to you under the License to provide to third parties,
for a fee or other consideration (including without limitation fees for hosting or consulting/ support services related to the Software),
a product or service whose value derives, entirely or substantially, from the functionality of the Software.
Any license notice or attribution required by the License must also include this Commons Clause License Condition notice.

Software: LocoConnect
License: GPLv3
Licensor: Roeland Kluit

Copyright (C) 2024 Roeland Kluit - v0.7 November 2026 - All rights reserved -

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The Software is provided to you by the Licensor under the License,
as defined, subject to the following condition.

Without limiting other conditions in the License, the grant of rights
under the License will not include, and the License does not grant to
you, the right to Sell the Software.

For purposes of the foregoing, "Sell" means practicing any or all of
the rights granted to you under the License to provide to third
parties, for a fee or other consideration (including without
limitation fees for hosting or consulting/ support services related
to the Software), a product or service whose value derives, entirely
or substantially, from the functionality of the Software.
Any license notice or attribution required by the License must also
include this Commons Clause License Condition notice.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#ifndef _LocoNetClassExt__
#define _LocoNetClassExt__

#include <LocoNet.h>
#include <utility/ln_sw_uart.h>

#define LOCONET_BUSY_WAIT_MS_MIN 4
#define LOCONET_BUSY_WAIT_MS_MAX 50

//Global var defined in loconet/utility/ln_sw_uart.cpp
extern volatile uint8_t lnState;

class LocoNetClassExt
{
private:
	static unsigned long LoconetBusyLastMillis;
	static uint16_t LoconetBusyWaitTime;

	//Return loconet uart idle state;
	static bool uartStateIdle()
	{
		uint8_t val;
		cli();
		val = lnState;
		sei();
		return (val == LN_ST_IDLE);
	}

public:
	/// <summary>
	///	Sets a busy wait time for the LocoNet interface.
	/// This function is used to indicate that the LocoNet interface is currently busy and should not be accessed until the specified wait time has passed.
	/// If no wait time is provided, a random wait time between LOCONET_BUSY_WAIT_MS_MIN and LOCONET_BUSY_WAIT_MS_MAX milliseconds will be set.
	/// The function also checks if a previous wait time has not yet passed and calculates the remaining wait time accordingly if this is longer than the new wait time.
	/// </summary>
	/// <param name="waitTime">The wait time in milliseconds</param>
	static void SetLoconetBusyWait(uint16_t waitTime = 0)
	{
		uint16_t currentWaitTime = 0;
		//If the previous wait time has not yet passed, calculate the remaining wait time
		if (LoconetBusyLastMillis != 0 && millis() - LoconetBusyLastMillis < LoconetBusyWaitTime)
		{
			//If the previous wait time has not yet passed, calculate the remaining wait time
			currentWaitTime = LoconetBusyWaitTime - (millis() - LoconetBusyLastMillis);
		}
		//Calculate the new wait time, either using the provided wait time or a random value between LOCONET_BUSY_WAIT_MS_MIN and LOCONET_BUSY_WAIT_MS_MAX
		uint16_t CalculatedWaitTime = waitTime != 0 ? waitTime : random(LOCONET_BUSY_WAIT_MS_MIN, LOCONET_BUSY_WAIT_MS_MAX);

		//Set the new wait time to the maximum of the current wait time and the calculated wait time
		if (CalculatedWaitTime > currentWaitTime)
		{
			LoconetBusyWaitTime = CalculatedWaitTime;
		}
		else
		{
			LoconetBusyWaitTime = currentWaitTime;
		}
		LoconetBusyLastMillis = millis();
	}

	/// <summary>
	/// Checks if the LocoNet interface is available for use.
	/// based on the availability of packets, the state of the UART, and any previously set busy wait time.
	/// </summary>
	/// <returns>true if the interface is available, false otherwise</returns>
	static bool CheckLoconetAvailable()
	{
		if (LocoNet.available())
		{
			//There is a packet available, so the interface is busy, wait for it to be processed
			return false;
		}
		else if (!LocoNetClassExt::uartStateIdle())
		{
			//The interface is busy, wait for it to be processed
			return false;
		}
		else if (LoconetBusyLastMillis != 0)
		{
			//Check if the wait time has passed, if not, wait for it to be processed
			if (millis() - LoconetBusyLastMillis < LoconetBusyWaitTime)
			{
				return false;
			}
			else
			{
				//The wait time has passed, reset the wait time and continue
				LoconetBusyLastMillis = 0;
				LoconetBusyWaitTime = 0;
			}
		}
		return true;
	}
};

unsigned long LocoNetClassExt::LoconetBusyLastMillis = 0;
uint16_t LocoNetClassExt::LoconetBusyWaitTime = 0;

#endif // _LocoNetClassExt__