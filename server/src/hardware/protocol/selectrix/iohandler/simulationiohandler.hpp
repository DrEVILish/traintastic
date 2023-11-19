/**
 * server/src/hardware/protocol/selectrix/iohandler/simulationiohandler.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2023 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_IOHANDLER_SIMULATIONIOHANDLER_HPP
#define TRAINTASTIC_SERVER_HARDWARE_PROTOCOL_SELECTRIX_IOHANDLER_SIMULATIONIOHANDLER_HPP

#include "iohandler.hpp"
#include <array>
#include "../bus.hpp"
#include "../const.hpp"

namespace Selectrix {

class SimulationIOHandler final : public IOHandler
{
  private:
    using BusValues = std::array<uint8_t, Address::max + 1>;
    Bus m_bus = Bus::SX0;
    std::array<BusValues, 3> m_busValues;

    inline BusValues& busValues()
    {
      return m_busValues[static_cast<uint8_t>(m_bus)];
    }

  public:
    SimulationIOHandler(Kernel& kernel);

    void start() final {}
    void stop() final {}

    bool read(uint8_t address, uint8_t& value) final;
    bool write(uint8_t address, uint8_t value) final;
};

template<>
constexpr bool isSimulation<SimulationIOHandler>()
{
  return true;
}

}

#endif

