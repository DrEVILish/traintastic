/**
 * server/src/hardware/protocol/traintasticdiy/inputstate.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_INPUTSTATE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_TRAINTASTICDIY_INPUTSTATE_HPP

#include <cstdint>
#include <string_view>

namespace TraintasticDIY {

enum class InputState : uint8_t
{
  Undefined = 0,
  False = 1,
  True = 2,
  Invalid = 3,
};

}

constexpr std::string_view toString(TraintasticDIY::InputState value)
{
  using InputState = TraintasticDIY::InputState;

  switch(value)
  {
    case InputState::Undefined:
      return "Undefined";

    case InputState::False:
      return "False";

    case InputState::True:
      return "True";

    case InputState::Invalid:
      return "Invalid";
  }
  return {};
}

#endif
