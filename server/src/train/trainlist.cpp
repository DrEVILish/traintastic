/**
 * server/src/vehicle/rail/trainlist.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2021 Reinder Feenstra
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

#include "trainlist.hpp"
#include "trainlisttablemodel.hpp"
#include "../world/world.hpp"
#include "../world/getworld.hpp"
#include "../core/attributes.hpp"
#include "../utils/displayname.hpp"

TrainList::TrainList(Object& _parent, std::string_view parentPropertyName) :
  ObjectList<Train>(_parent, parentPropertyName),
  add{*this, "add",
    [this]()
    {
      auto world = getWorld(&this->parent());
      if(!world)
        return std::shared_ptr<Train>();
      return Train::create(world, world->getUniqueId("train"));
    }}
  , remove{*this, "remove",
      [this](const std::shared_ptr<Train>& train)
      {
        if(containsObject(train))
          train->destroy();
        assert(!containsObject(train));
      }}
{
  auto world = getWorld(&_parent);
  const bool editable = world && contains(world->state.value(), WorldState::Edit);

  Attributes::addDisplayName(add, DisplayName::List::add);
  Attributes::addEnabled(add, editable);
  m_interfaceItems.add(add);

  Attributes::addDisplayName(remove, DisplayName::List::remove);
  Attributes::addEnabled(remove, editable);
  m_interfaceItems.add(remove);
}

TableModelPtr TrainList::getModel()
{
  return std::make_shared<TrainListTableModel>(*this);
}

void TrainList::worldEvent(WorldState state, WorldEvent event)
{
  ObjectList<Train>::worldEvent(state, event);

  const bool editable = contains(state, WorldState::Edit);

  Attributes::setEnabled(add, editable);
  Attributes::setEnabled(remove, editable);
}

bool TrainList::isListedProperty(std::string_view name)
{
  return TrainListTableModel::isListedProperty(name);
}
