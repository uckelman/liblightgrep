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
#include "graph.h"
#include "transition.h"
#include "transitionfactory.h"
#include "vectorfamily.h"

#include <ostream>
#include <string>

struct Properties {
  Properties(): Deterministic(true), TransFac(new TransitionFactory()) {}

  bool Deterministic;
  std::shared_ptr<TransitionFactory> TransFac;
};

struct GlushkovState {
  static const uint32_t NOLABEL;

  GlushkovState():
    Trans(nullptr), Label(NOLABEL), IsMatch(false),
    AtStart(false), AtEnd(false), Assert(false) {}

  std::string label() const;

  Transition* Trans;
  uint32_t Label;
// FIXME: Maybe combine the flags into a uint8_t?
  bool IsMatch;
  bool AtStart;
  bool AtEnd;
  bool Assert;
};

struct Empty {};

using NFA = Graph<Properties,GlushkovState,Empty,VectorFamily>;

template <>
void writeVertex<Properties,GlushkovState,Empty,VectorFamily>(std::ostream& out, typename NFA::VertexDescriptor v, const NFA& g);

template <>
void writeEdge<Properties,GlushkovState,Empty,VectorFamily>(std::ostream& out, typename NFA::VertexDescriptor v, typename NFA::VertexDescriptor u, uint32_t index, const NFA& g);
