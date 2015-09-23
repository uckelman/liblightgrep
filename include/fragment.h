/*
  liblightgrep: not the worst forensics regexp engine
  Copyright (C) 2013, Lightbox Technologies, Inc

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "basic.h"
#include "automata.h"
#include "parsenode.h"

#include <vector>

using InListT = std::vector<NFA::VertexDescriptor>;
using OutListT = std::vector<std::pair<NFA::VertexDescriptor, uint32_t>>;

static const uint32_t NOSKIP = std::numeric_limits<uint32_t>::max();

struct Fragment {
  Fragment(): Skippable(NOSKIP) {}

  Fragment(NFA::VertexDescriptor in, const ParseNode& n):
    InList(1, in), N(n), Skippable(NOSKIP) {}

  /*
   * InList is the list of vertices in this fragment which have incoming
   * edges from outside the fragment. OutList is the is the list of vertices
   * in this fragment which have edges leaving the fragment.
   */
  InListT InList;
  OutListT OutList;
  ParseNode N;

  uint32_t Skippable;

  void initFull(NFA::VertexDescriptor in, const ParseNode& n) {
    N = n;
    Skippable = NOSKIP;
    InList.clear();
    InList.push_back(in);
    OutList.clear();
    OutList.emplace_back(in, 0);
  }

  void reset(const ParseNode& n) {
    N = n;
    Skippable = NOSKIP;
    InList.clear();
    OutList.clear();
  }

  void assign(Fragment& f) {
    InList.swap(f.InList);
    OutList.swap(f.OutList);
    N = f.N;
    Skippable = f.Skippable;
  }
};
