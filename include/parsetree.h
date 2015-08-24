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
#include "parsenode.h"

#include <iosfwd>
#include <stdexcept>
#include <vector>

class ParseTree {
public:
  ParseNode* Root;

  ParseTree(): Root(nullptr) {}

  ParseTree(const ParseTree&) = delete;

  ParseTree(ParseTree&&) = default;

  ParseTree(const ParseNode*);

  ParseTree& operator=(const ParseTree&) = delete;

  ParseTree& operator=(ParseTree&&) = default;

  ParseNode* copySubtree(const ParseNode* src);

  template <class... Args>
  ParseNode* add(Args&&... args) {
    Store.emplace_back(std::forward<Args>(args)...);
    return &Store[Store.size()-1];
  }

  //
  // Sizing explanation:
  //
  // * Every character in a pattern contributes at most one node to
  // the parse tree. Some characters, such as parentheses, the square
  // brackets for character classes, and the nongreedy marker '?'
  // contribute none.
  //
  // * Concatenation is implicit in patterns. Each intercharacter
  // position potentially contributes one node to the parse tree.
  //
  // * The root is one node in the parse tree.
  //
  // The worst case is a pattern made up of n literals, which will
  // generate n nodes for the literals, n-1 nodes for the concatenations,
  // and one node for the root. n + n - 1 + 1 = 2n.
  //
  // Therefore, sizing the vector to twice the length of the pattern
  // ensures that the vector will never resize on us and invalidate our
  // ParseNode pointers.
  //
  void init(size_t len) {
    Root = nullptr;
    Store.clear();
    Store.reserve(2*len);
  }

  bool expand(size_t size);

  // NB: It is possible for size() > 0 && empty(). This is because empty()
  // is about whether the object contains a valid tree, while size() is
  // about the number of nodes stored.

  size_t size() const {
    return Store.size();
  }

  bool empty() const {
    return !Root || !Root->Child.Left;
  }

  bool operator==(const ParseTree& other) const {
    return subtreeCompare(Root, other.Root);
  }

private:
  std::vector<ParseNode> Store;
};

std::ostream& operator<<(std::ostream& out, const ParseTree& tree);
