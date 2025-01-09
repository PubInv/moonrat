/* \_utility.h
  Header files for low-level utility functions needed by every module (presumably) in the Moonrat System

  Copyright (C) 2022 Robert Read

  This program includes free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  See the GNU Affero General Public License for more details.
  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/

#ifndef UTILITY
#define UTILITY 1

#define PROG_NAME "MoonRat "
#define FIRMWARE_VERSION "0.3 "
#define MODEL_NAME "MOONRAT"
#define LICENSE "GNU Affero General Public License, version 3 "
#define ORIGIN "US"

#define VOLTAGE_V 12.0
#define RESISTANCE_OHMS 6.3
#define POWER_WATTS (VOLTAGE_V * VOLTAGE_V / RESISTANCE_OHMS) 

#define MS_TO_HOURS (1.0 / (1000.0 * 60.0 * 60.0))

#endif
