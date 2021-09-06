/**
 * server/test/board/addtile.cpp
 *
 * This file is part of the traintastic test suite.
 *
 * Copyright (C) 2021 Reinder Feenstra
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

#include <catch2/catch.hpp>
#include "../src/world/world.hpp"
#include "../src/board/board.hpp"

#include "../src/board/tile/rail/straightrailtile.hpp"
#include "../src/board/tile/rail/curve45railtile.hpp"
#include "../src/board/tile/rail/curve90railtile.hpp"
#include "../src/board/tile/rail/cross45railtile.hpp"
#include "../src/board/tile/rail/cross90railtile.hpp"
#include "../src/board/tile/rail/bridge45leftrailtile.hpp"
#include "../src/board/tile/rail/bridge45rightrailtile.hpp"
#include "../src/board/tile/rail/bridge90railtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutleft45railtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutleft90railtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutleftcurvedrailtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutright45railtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutright90railtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutrightcurvedrailtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutwyerailtile.hpp"
#include "../src/board/tile/rail/turnout/turnout3wayrailtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutsinglesliprailtile.hpp"
#include "../src/board/tile/rail/turnout/turnoutdoublesliprailtile.hpp"
#include "../src/board/tile/rail/signal/signal2aspectrailtile.hpp"
#include "../src/board/tile/rail/signal/signal3aspectrailtile.hpp"
#include "../src/board/tile/rail/bufferstoprailtile.hpp"
#include "../src/board/tile/rail/sensorrailtile.hpp"
#include "../src/board/tile/rail/blockrailtile.hpp"
#include "../src/board/tile/rail/tunnelrailtile.hpp"

TEST_CASE("Board: Add non existing tile", "[board][board-add]")
{
  auto world = World::create();
  auto board = world->boards->add();

  REQUIRE_FALSE(board->addTile(0, 0, TileRotate::Deg0, "board_tile.i_n_v_a_l_i_d", false));
}

TEMPLATE_TEST_CASE("Board: Add tile", "[board][board-add]"
  , StraightRailTile
  , Curve45RailTile
  , Curve90RailTile
  , Cross45RailTile
  , Cross90RailTile
  , Bridge45LeftRailTile
  , Bridge45RightRailTile
  , Bridge90RailTile
  , TurnoutLeft45RailTile
  , TurnoutLeft90RailTile
  , TurnoutLeftCurvedRailTile
  , TurnoutRight45RailTile
  , TurnoutRight90RailTile
  , TurnoutRightCurvedRailTile
  , TurnoutWyeRailTile
  , Turnout3WayRailTile
  , TurnoutSingleSlipRailTile
  , TurnoutDoubleSlipRailTile
  , Signal2AspectRailTile
  , Signal3AspectRailTile
  , BufferStopRailTile
  , SensorRailTile
  , BlockRailTile
  , TunnelRailTile
  )
{
  auto world = World::create();
  auto board = world->boards->add();

  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, TestType::classId, false));

  REQUIRE_FALSE(board->addTile(0, 0, TileRotate::Deg0, TestType::classId, false));

  REQUIRE(board->addTile(0, 0, TileRotate::Deg0, TestType::classId, true));
}