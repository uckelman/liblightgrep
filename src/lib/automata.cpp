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

#include "automata.h"

#include <limits>
#include <string>

const uint32_t GlushkovState::NOLABEL = std::numeric_limits<uint32_t>::max();

std::string GlushkovState::label() const {
  std::ostringstream buf;
  if (Trans) {
    buf << Trans->label();
    if (Label != NOLABEL) {
      buf << "/" << Label;
    }
  }
  return buf.str();
}

template <>
void writeVertex<Properties,GlushkovState,Empty,VectorFamily>(std::ostream& out, typename NFA::VertexDescriptor v, const NFA& g) {
  out << "  " << v << " [label=\"" << v << "\"";

  if (g[v].IsMatch) {
    // double ring for match states
    out << ", peripheries=2";
  }

  out << "];\n";
}

namespace {

std::string escape(char c, const std::string& text) {
  // escape a character in the given string
  std::string repl(text);
  for (std::string::size_type next = repl.find(c);
       next != std::string::npos; next = repl.find(c, next)) {
    repl.insert(next, 1, '\\');
    next += 2;
  }
  return repl;
}

}

template <>
void writeEdge<Properties,GlushkovState,Empty,VectorFamily>(std::ostream& out, typename NFA::VertexDescriptor v, typename NFA::VertexDescriptor u, uint32_t index, const NFA& g) {
  out << "  " << v << " -> " << u << " ["
      << "label=\"" << escape('"', escape('\\', g[u].label())) << "\", "
      << "taillabel=\"" << index << "\"];\n";
}
