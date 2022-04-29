/**
 * shared/src/enum/interfacestatus.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INTERFACESTATUS_HPP
#define TRAINTASTIC_SHARED_TRAINTASTIC_ENUM_INTERFACESTATUS_HPP

#include <cstdint>
#include "enum.hpp"

enum class InterfaceStatus : uint8_t
{
  Offline = 0,
  Initializing = 1,
  Online = 2,
  Error = 255,
};

TRAINTASTIC_ENUM(InterfaceStatus, "interface_status", 4,
{
  {InterfaceStatus::Offline, "offline"},
  {InterfaceStatus::Initializing, "initializing"},
  {InterfaceStatus::Online, "online"},
  {InterfaceStatus::Error, "error"},
});

#endif
