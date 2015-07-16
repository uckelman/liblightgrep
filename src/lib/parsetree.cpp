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

#include "parsetree.h"

#include <ostream>

void ParseTree::init(size_t len) {
  Root = nullptr;
  Store.clear();
  Store.reserve(2*len);
}

bool ParseTree::expand(size_t len) {
  if (len > Store.capacity()) {
    // Expanding the Store will invalidate all of our pointers, but their
    // offsets relative to each other are still valid, so we can rebase
    // them against the new first element.
    const ParseNode* const oldbase = &Store[0];
    Store.reserve(len);
    const off_t delta = &Store[0] - oldbase;

    Root += delta;

    for (ParseNode& n: Store) {
      switch (n.Type) {
      case ParseNode::ALTERNATION:
      case ParseNode::CONCATENATION:
        n.Child.Right += delta;

      case ParseNode::REGEXP:
      case ParseNode::LOOKBEHIND_POS:
      case ParseNode::LOOKBEHIND_NEG:
      case ParseNode::LOOKAHEAD_POS:
      case ParseNode::LOOKAHEAD_NEG:
      case ParseNode::REPETITION:
      case ParseNode::REPETITION_NG:
        n.Child.Left += delta;
        break;

      default:
        break;
      }
    }

    return true;
  }
  else {
    return false;
  }
}

void printTree(std::ostream& out, const ParseNode& n) {
  switch (n.Type) {
  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    if (n.Child.Right) {
      printTree(out, *n.Child.Right);
    }
  case ParseNode::REGEXP:
  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    if (n.Child.Left) {
      printTree(out, *n.Child.Left);
    }
    break;
  default:
    break;
  }

  out << n << '\n';
}

std::ostream& operator<<(std::ostream& out, const ParseTree& tree) {
  if (tree.Root) {
    printTree(out, *tree.Root);
  }
  return out;
}
