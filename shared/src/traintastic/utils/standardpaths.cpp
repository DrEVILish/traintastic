/**
 * shared/src/traintastic/utils/standardpaths.cpp
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

#include "standardpaths.hpp"
#ifdef WIN32
  #include <windows.h>
  #include <shlobj.h>
#endif

#ifdef WIN32
std::filesystem::path getLocalAppDataPath()
{
  std::filesystem::path path;
  PWSTR localAppDataPath = nullptr;
  HRESULT r = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppDataPath);
  if(r == S_OK)
    path = std::filesystem::path(localAppDataPath);
  if(localAppDataPath)
    CoTaskMemFree(localAppDataPath);
  return path;
}
#endif

std::filesystem::path getLocalePath()
{
  if(const char* path = getenv("TRAINTASTIC_LOCALE_PATH"))
    return std::filesystem::path(path);

#ifdef WIN32
  return getLocalAppDataPath() / "traintastic" / "shared" / "lang";
#elif defined(__linux__)
  return "/opt/traintastic/lang";
#else
  return std::filesystem::current_path() / "lang";
#endif
}
