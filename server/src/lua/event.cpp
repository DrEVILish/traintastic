/**
 * server/src/lua/event.cpp - Lua event wrapper
 *
 * This file is part of the traintastic source code.
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

#include "event.hpp"
#include "to.hpp"
#include "checkarguments.hpp"
#include "error.hpp"
#include "eventhandler.hpp"
#include "sandbox.hpp"
#include "../core/abstractevent.hpp"
#include "../core/object.hpp"

namespace Lua {

struct EventData
{
  ObjectPtrWeak object;
  AbstractEvent& event;

  EventData(AbstractEvent& _event) :
    object{_event.object().weak_from_this()},
    event{_event}
  {
  }
};

AbstractEvent& Event::check(lua_State* L, int index)
{
  EventData& data = **static_cast<EventData**>(luaL_checkudata(L, index, metaTableName));
  if(!data.object.expired())
    return data.event;

  errorDeadEvent(L);
}

AbstractEvent* Event::test(lua_State* L, int index)
{
  EventData** data = static_cast<EventData**>(luaL_testudata(L, index, metaTableName));
  if(!data)
    return nullptr;
  if(!(**data).object.expired())
    return &(**data).event;

  errorDeadEvent(L);
}

void Event::push(lua_State* L, AbstractEvent& value)
{
  *static_cast<EventData**>(lua_newuserdata(L, sizeof(EventData*))) = new EventData(value);
  luaL_getmetatable(L, metaTableName);
  lua_setmetatable(L, -2);
}

void Event::registerType(lua_State* L)
{
  luaL_newmetatable(L, metaTableName);
  lua_pushcfunction(L, __index);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, __gc);
  lua_setfield(L, -2, "__gc");
  lua_pop(L, 1);
}

int Event::__index(lua_State* L)
{
  auto& event = check(L, 1);
  auto name = to<std::string_view>(L, 2);

  if(name == "connect")
  {
    push(L, event);
    lua_pushcclosure(L, connect, 1);
  }
  else if(name == "disconnect")
  {
    push(L, event);
    lua_pushcclosure(L, disconnect, 1);
  }
  else
    lua_pushnil(L);

  return 1;
}

int Event::__gc(lua_State* L)
{
  delete *static_cast<EventData**>(lua_touserdata(L, 1));
  return 0;
}

int Event::connect(lua_State* L)
{
  checkArguments(L, 1, 2);

  auto& event = check(L, lua_upvalueindex(1));
  auto handler = std::make_shared<EventHandler>(event, L);
  event.connect(handler);
  lua_pushinteger(L, Sandbox::getStateData(L).registerEventHandler(handler));

  return 1;
}

int Event::disconnect(lua_State* L)
{
  checkArguments(L, 1);

  auto& event = check(L, lua_upvalueindex(1));
  auto handler = Sandbox::getStateData(L).getEventHandler(luaL_checkinteger(L, 1));

  lua_pushboolean(L, handler && &handler->event() == &event && handler->disconnect());

  return 1;
}

}
