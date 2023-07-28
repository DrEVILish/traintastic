/**
 * server/src/utils/zlib.cpp
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

#include "zlib.hpp"
#include <zlib.h>

namespace ZLib::Uncompress {

bool toString(const void* src, size_t srcSize, size_t dstSize, std::string& out)
{
  out.resize(dstSize);
  int r = uncompress(reinterpret_cast<Bytef*>(out.data()), &dstSize, reinterpret_cast<const Bytef*>(src), srcSize);
  out.resize(dstSize);
  return r == Z_OK;
}

}
