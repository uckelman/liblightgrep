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

#include "rewriter.h"

#include <iostream>
#include <map>
#include <stack>

#include <boost/lexical_cast.hpp>

void print_branch(std::ostream& out, std::stack<ParseNode*>& branch) {
  std::stack<ParseNode*> tmp;

  while (!branch.empty()) {
    tmp.push(branch.top());
    branch.pop();
  }

  ParseNode* n;
  while (!tmp.empty()) {
    n = tmp.top();
    out << *n << '\n';
    branch.push(n);
    tmp.pop();
  }
}

void spliceOutParent(ParseNode* gp, const ParseNode* p, ParseNode* c) {
  if (gp->Child.Left == p) {
    gp->Child.Left = c;
  }
  else if (gp->Child.Right == p) {
    gp->Child.Right = c;
  }
  else {
    throw std::logic_error("wtf");
  }
}

bool hasZeroLengthMatch(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return !n->Child.Left || hasZeroLengthMatch(n->Child.Left);

  case ParseNode::ALTERNATION:
    return hasZeroLengthMatch(n->Child.Left) || hasZeroLengthMatch(n->Child.Right);

  case ParseNode::CONCATENATION:
    return hasZeroLengthMatch(n->Child.Left) && hasZeroLengthMatch(n->Child.Right);

  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    return n->Child.Rep.Min == 0 || hasZeroLengthMatch(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return false;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

bool prefers_zero_length_match(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return !n->Child.Left || prefers_zero_length_match(n->Child.Left);

  case ParseNode::ALTERNATION:
    // Left has priority, so we don't need to check right.
    return prefers_zero_length_match(n->Child.Left);

  case ParseNode::CONCATENATION:
    return prefers_zero_length_match(n->Child.Left) &&
           prefers_zero_length_match(n->Child.Right);

  case ParseNode::REPETITION:
    return n->Child.Rep.Max == 0 || prefers_zero_length_match(n->Child.Left);

  case ParseNode::REPETITION_NG:
    return n->Child.Rep.Min == 0 || prefers_zero_length_match(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return false;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

bool has_only_zero_length_match(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return !n->Child.Left || has_only_zero_length_match(n->Child.Left);

  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
    // Don't apply this to assertions.
    return false;

  case ParseNode::ALTERNATION:
    // Left has priority, so we don't need to check right.
    return has_only_zero_length_match(n->Child.Left);

  case ParseNode::CONCATENATION:
    return has_only_zero_length_match(n->Child.Left) &&
           has_only_zero_length_match(n->Child.Right);

  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    return (n->Child.Rep.Min == 0 && n->Child.Rep.Max == 0) ||
           has_only_zero_length_match(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return false;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

bool reduceEmptySubtrees(ParseNode* n, std::stack<ParseNode*>& branch) {

  // ST{0}Q = ST{0}?Q = T{0}QS = T{0}?Q = S
  // R(S{0}Q|T) = (S{0}Q|T)R = R
  // (S|T{0}Q) = S?

  bool ret = false;
  branch.push(n);

  if (has_only_zero_length_match(n)) {
    switch (n->Type) {
    case ParseNode::REGEXP:
      // prune the whole tree
      n->Child.Left = 0;
      break;

    case ParseNode::ALTERNATION:
    case ParseNode::CONCATENATION:
    case ParseNode::REPETITION:
    case ParseNode::REPETITION_NG:
      // replace this subtree with a dummy
      n->setType(ParseNode::REPETITION);
      n->Child.Rep.Min = n->Child.Rep.Max = 0;

      // this is safe---we know that n must have a left child if it is
      // not the root and has a zero length match
      n->Child.Left->setType(ParseNode::LITERAL);
      n->Child.Left->Child.Left = n->Child.Left->Child.Right = 0;
      n->Child.Left->Val = 'x';
      break;

    default:
      // WTF?
      throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
    }
    ret = true;
  }
  else {
    switch (n->Type) {
    case ParseNode::REGEXP:
    case ParseNode::LOOKAHEAD_POS:
    case ParseNode::LOOKAHEAD_NEG:
    case ParseNode::LOOKBEHIND_POS:
    case ParseNode::LOOKBEHIND_NEG:
    case ParseNode::REPETITION:
    case ParseNode::REPETITION_NG:
      ret = reduceEmptySubtrees(n->Child.Left, branch);
      break;

    case ParseNode::ALTERNATION:
    case ParseNode::CONCATENATION:
      ret = reduceEmptySubtrees(n->Child.Left, branch);
      ret |= reduceEmptySubtrees(n->Child.Right, branch);
      break;

    case ParseNode::DOT:
    case ParseNode::CHAR_CLASS:
    case ParseNode::LITERAL:
    case ParseNode::BYTE:
      // branch finished
      ret = false;
      break;

    default:
      // WTF?
      throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
    }
  }

  branch.pop();

  // fix concatenations and alternations with dead children on the way up
  if (n->Type == ParseNode::CONCATENATION) {
    // convert ST{0} and T{0}S into S
    if (has_only_zero_length_match(n->Child.Left)) {
      spliceOutParent(branch.top(), n, n->Child.Right);
    }
    else if (has_only_zero_length_match(n->Child.Right)) {
      spliceOutParent(branch.top(), n, n->Child.Left);
    }
  }
  else if (n->Type == ParseNode::ALTERNATION) {
    if (has_only_zero_length_match(n->Child.Right)) {
      // convert S|T{0} into S?
      n->setType(ParseNode::REPETITION);
      n->Child.Rep.Min = 0;
      n->Child.Rep.Max = 1;
    }
  }

  return ret;
}

bool reduceEmptySubtrees(ParseNode* root) {
  std::stack<ParseNode*> branch;
  return reduceEmptySubtrees(root, branch);
}

bool prune_useless_repetitions(ParseNode* n, const std::stack<ParseNode*>& branch) {
  if ((n->Type == ParseNode::REPETITION ||
       n->Type == ParseNode::REPETITION_NG) &&
      n->Child.Rep.Min == 1 && n->Child.Rep.Max == 1) {
    // remove {1,1}, {1,1}?
    ParseNode* parent = branch.top();
    if (n == parent->Child.Left) {
      parent->Child.Left = n->Child.Left;
    }
    else {
      parent->Child.Right = n->Child.Left;
    }

    // recurse, to handle consecutive repetitions
    prune_useless_repetitions(n->Child.Left, branch);
    return true;
  }
  else if (n->Type == ParseNode::REPETITION_NG &&
           n->Child.Rep.Min == n->Child.Rep.Max) {
    // reduce {n}? to {n}
    n->setType(ParseNode::REPETITION);
    return true;
  }

  return false;
}

bool reduceUselessRepetitions(ParseNode* n, std::stack<ParseNode*>& branch) {
  // T{1} = T{1}? = T
  // T{n}? = T{n}

  bool ret = false;
  branch.push(n);

  switch (n->Type) {
  case ParseNode::REGEXP:
    if (!n->Child.Left) {
      return ret;
    }
  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    ret = prune_useless_repetitions(n->Child.Left, branch);
    ret |= reduceUselessRepetitions(n->Child.Left, branch);
    break;

  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    ret = prune_useless_repetitions(n->Child.Left, branch);
    ret |= reduceUselessRepetitions(n->Child.Left, branch);
    ret |= prune_useless_repetitions(n->Child.Right, branch);
    ret |= reduceUselessRepetitions(n->Child.Right, branch);
    break;

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    // branch finished
    ret = false;
    break;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }

  branch.pop();
  return ret;
}

bool reduceUselessRepetitions(ParseNode* root) {
  std::stack<ParseNode*> branch;
  return reduceUselessRepetitions(root, branch);
}

bool combinable(ParseNode* x, ParseNode* y) {
  return
  (
    (x->Type == ParseNode::REPETITION && y->Type == ParseNode::REPETITION) ||
    (x->Type == ParseNode::REPETITION_NG && y->Type == ParseNode::REPETITION_NG)
  ) && *x == *y;
}

bool combineConsecutiveRepetitions(ParseNode* n, std::stack<ParseNode*>& branch) {
  // T{a,b}T{c,d} == T{a+c,b+d}
  // T{a,b}?T{c,d}? == T{a+c,b+d}?

  bool ret = false;
  branch.push(n);

  switch (n->Type) {
  case ParseNode::REGEXP:
    if (!n->Child.Left) {
      return ret;
    }
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    ret = combineConsecutiveRepetitions(n->Child.Left, branch);
    break;

  case ParseNode::ALTERNATION:
    ret = combineConsecutiveRepetitions(n->Child.Left, branch);
    ret |= combineConsecutiveRepetitions(n->Child.Right, branch);
    break;

  case ParseNode::CONCATENATION:
    ret = combineConsecutiveRepetitions(n->Child.Left, branch);
    ret |= combineConsecutiveRepetitions(n->Child.Right, branch);

    if (combinable(n->Child.Left, n->Child.Right)) {
      // the repetitions are siblings
      n->Child.Left->Child.Rep.Min += n->Child.Right->Child.Rep.Min;
      n->Child.Left->Child.Rep.Max =
        (n->Child.Left->Child.Rep.Max == UNBOUNDED || n->Child.Right->Child.Rep.Max == UNBOUNDED)
        ? UNBOUNDED : n->Child.Left->Child.Rep.Max + n->Child.Right->Child.Rep.Max;

      branch.pop();
      spliceOutParent(branch.top(), n, n->Child.Left);
      branch.push(n->Child.Left);
      ret = true;
    }
    else if (n->Child.Right->Type == ParseNode::CONCATENATION &&
             combinable(n->Child.Left, n->Child.Right->Child.Left)) {
      // the second repetition is the left nephew of the first
      n->Child.Left->Child.Rep.Min += n->Child.Right->Child.Left->Child.Rep.Min;
      n->Child.Left->Child.Rep.Max =
        (n->Child.Left->Child.Rep.Max == UNBOUNDED || n->Child.Right->Child.Left->Child.Rep.Max == UNBOUNDED)
        ? UNBOUNDED : n->Child.Left->Child.Rep.Max + n->Child.Right->Child.Left->Child.Rep.Max;
      n->Child.Right = n->Child.Right->Child.Right;
      ret = true;
    }
    break;

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    // branch finished
    ret = false;
    break;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }

  branch.pop();
  return ret;
}

bool combineConsecutiveRepetitions(ParseNode* root) {
  std::stack<ParseNode*> branch;
  return combineConsecutiveRepetitions(root, branch);
}

void gatherXjuncts(ParseNode* n, ParseNode::NodeType type, std::vector<ParseNode*>& leaves, std::vector<ParseNode*>& ops) {
  if (n->Type == type) {
    ops.push_back(n);
    gatherXjuncts(n->Child.Left, type, leaves, ops);
    gatherXjuncts(n->Child.Right, type, leaves, ops);
  }
  else {
    leaves.push_back(n);
  }
}

bool makeBinopsRightAssociative(ParseNode* n, std::stack<ParseNode*>& branch) {
  bool ret = false;
  branch.push(n);

  switch (n->Type) {
  case ParseNode::REGEXP:
    if (!n->Child.Left) {
      return ret;
    }
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
    ret = makeBinopsRightAssociative(n->Child.Left, branch);
    break;

  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    {
      branch.pop();
      ParseNode* p = branch.top();
      branch.push(n);

      if (p->Type != n->Type) {
        /*
          Adjust consecutive binary nodes so that consecutive same-type
          binary ops are the right children of their parents.

          Skip if the parent of this node is the same type, as we will
          already have processed this node when we met its parent.
        */
        std::vector<ParseNode*> leaves, ops;
        gatherXjuncts(n, n->Type, leaves, ops);

        if (ops.size() > 1) {
          auto o = ops.begin();
          auto l = leaves.begin();

          // add each of the left leaves, working left to right
          while (true) {
            if ((*o)->Child.Left != *l) {
              (*o)->Child.Left = *l;
              ret = true;
            }
            ++l;

            if (o + 1 == ops.end()) {
              break;
            }

            if ((*o)->Child.Right != *(o+1)) {
              (*o)->Child.Right = *(o+1);
              ret = true;
            }
            ++o;
          }

          // add the sole right leaf
          if ((*o)->Child.Right != *l) {
            (*o)->Child.Right = *l;
            ret = true;
          }
        }
      }

      ret |= makeBinopsRightAssociative(n->Child.Left, branch);
      ret |= makeBinopsRightAssociative(n->Child.Right, branch);
    }
    break;

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    // branch finished
    ret = false;
    break;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }

  branch.pop();
  return ret;
}

bool makeBinopsRightAssociative(ParseNode* root) {
  std::stack<ParseNode*> branch;
  return makeBinopsRightAssociative(root, branch);
}

bool reduceTrailingNongreedyThenGreedy(ParseNode* n, std::stack<ParseNode*>& branch) {
  // T{a,b}?T{c,d} == T{a+c,a+d}

  bool ret = false;
  branch.push(n);

  switch (n->Type) {
  case ParseNode::REGEXP:
    if (!n->Child.Left) {
      return ret;
    }
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    ret = reduceTrailingNongreedyThenGreedy(n->Child.Left, branch);
    break;

  case ParseNode::ALTERNATION:
    ret = reduceTrailingNongreedyThenGreedy(n->Child.Left, branch);
    ret |= reduceTrailingNongreedyThenGreedy(n->Child.Right, branch);
    break;

  case ParseNode::CONCATENATION:
    ret = reduceTrailingNongreedyThenGreedy(n->Child.Right, branch);

    if (n->Child.Left->Type == ParseNode::REPETITION_NG) {
      if (n->Child.Right->Type == ParseNode::REPETITION &&
          *n->Child.Left->Child.Left == *n->Child.Right->Child.Left) {
        const uint32_t a = n->Child.Left->Child.Rep.Min;
        const uint32_t c = n->Child.Right->Child.Rep.Min;
        const uint32_t d = n->Child.Right->Child.Rep.Max;

        n->Child.Left->setType(ParseNode::REPETITION);
        n->Child.Left->Child.Rep.Min = a + c;
        n->Child.Left->Child.Rep.Max = d == UNBOUNDED ? UNBOUNDED : a + d;

        branch.pop();
        spliceOutParent(branch.top(), n, n->Child.Left);
        reduceTrailingNongreedyThenGreedy(n->Child.Left, branch);
        branch.push(n->Child.Left);

        ret = true;
      }
      else if (*n->Child.Left->Child.Left == *n->Child.Right) {
        const uint32_t a = n->Child.Left->Child.Rep.Min;
        const uint32_t c = 1;
        const uint32_t d = 1;

        n->Child.Left->setType(ParseNode::REPETITION);
        n->Child.Left->Child.Rep.Min = a + c;
        n->Child.Left->Child.Rep.Max = a + d;

        branch.pop();
        spliceOutParent(branch.top(), n, n->Child.Left);
        reduceTrailingNongreedyThenGreedy(n->Child.Left, branch);
        branch.push(n->Child.Left);
        ret = true;
      }
    }
    break;

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    // branch finished
    ret = false;
    break;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }

  branch.pop();
  return ret;
}

bool reduceTrailingNongreedyThenGreedy(ParseNode* root) {
  std::stack<ParseNode*> branch;
  return reduceTrailingNongreedyThenGreedy(root, branch);
}

bool reduceTrailingNongreedyThenEmpty(ParseNode* n, std::stack<ParseNode*>& branch) {
  /*
     As a suffix, S{n,m}?T = S{n}T, when T admits zero-length matches.

     In the tree, the adjacency can show up as either S{n,m}? and T as
     children of the same concatenation, or as T being the right uncle
     of S{n,m}?:

         &            &
        / \    OR    / \
       +?  T        &   T
        |          / \
        S            +?
                      |
                      S

     As a suffix, S{n,m}? = S{n}. This is a special case of the above,
     letting T = R{0}.

     However, the second tree is impossible if we first rewrite trees
     to be right-associative for contatenation.
  */

  bool ret = false;
  branch.push(n);

  switch (n->Type) {
  case ParseNode::REGEXP:
    if (!n->Child.Left) {
      return ret;
    }
  case ParseNode::REPETITION:
    ret = reduceTrailingNongreedyThenEmpty(n->Child.Left, branch);
    break;

  case ParseNode::REPETITION_NG:
    // replace S{n,m}? with S{n}
    n->setType(ParseNode::REPETITION);
    n->Child.Rep.Max = n->Child.Rep.Min;
    reduceTrailingNongreedyThenEmpty(n->Child.Left, branch);
    ret = true;
    break;

  case ParseNode::ALTERNATION:
    ret = reduceTrailingNongreedyThenEmpty(n->Child.Left, branch);
    ret |= reduceTrailingNongreedyThenEmpty(n->Child.Right, branch);
    break;

  case ParseNode::CONCATENATION:
    if (hasZeroLengthMatch(n->Child.Right)) {
      if (n->Child.Left->Type == ParseNode::REPETITION_NG) {
        // the left child is S{n,m}?, the right child is T

        // replace S{n,m}? with S{n}
        n->Child.Left->setType(ParseNode::REPETITION);
        n->Child.Left->Child.Rep.Max = n->Child.Left->Child.Rep.Min;

        ret = true;
      }
      else {
        // check the left, it is trailed by an empty-matching subpattern
        ret = reduceTrailingNongreedyThenEmpty(n->Child.Left, branch);
      }
    }

    ret |= reduceTrailingNongreedyThenEmpty(n->Child.Right, branch);
    break;

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    // branch finished
    ret = false;
    break;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }

  branch.pop();
  return ret;
}

bool reduceTrailingNongreedyThenEmpty(ParseNode* root) {
  std::stack<ParseNode*> branch;
  return reduceTrailingNongreedyThenEmpty(root, branch);
}

void reduceNegativeLookbehindLiteral(ParseNode* n, ParseTree& tree) {
  // (?<![l]) => \A|(?<=[^l])

  ParseNode* l = n->Child.Left;

  if (l->Type == ParseNode::DOT) {
    // this is an \A, do nothing
    return;
  }

  n->Type = ParseNode::ALTERNATION;
  n->Child.Left =
    tree.add(ParseNode::LOOKBEHIND_NEG, tree.add(ParseNode::DOT, '.'));

  switch (l->Type) {
  case ParseNode::CHAR_CLASS:
// FIXME: this is probably wrong in the presence of breakout bytes
    l->Set.CodePoints.flip();
    break;

  case ParseNode::LITERAL:
    *l = ParseNode(ParseNode::CHAR_CLASS, ~UnicodeSet(l->Val));
    break;

  case ParseNode::BYTE:
    *l = ParseNode(ParseNode::CHAR_CLASS, UnicodeSet(), ~ByteSet(l->Val), true);
    break;

  default:
    throw std::logic_error("wtf");
  }

  n->Child.Right = tree.add(ParseNode::LOOKBEHIND_POS, l);
}

void reduceNegativeLookbehindConcatenation(ParseNode* n, ParseTree& tree) {
  // (?<!ST) => (?<!T)|(?<=(?<!S)T)
  ParseNode* c = n->Child.Left;
  ParseNode* s = c->Child.Left;
  ParseNode* t = c->Child.Right;

  n->Type = ParseNode::ALTERNATION;

  c->Type = ParseNode::LOOKBEHIND_NEG;
  c->Child.Left = t;

  n->Child.Right = tree.add(ParseNode::LOOKBEHIND_POS,
    tree.add(ParseNode::CONCATENATION,
      tree.add(ParseNode::LOOKBEHIND_NEG, s),
      tree.copySubtree(t)
    )
  );
}

void reduceNegativeLookbehindAlternation(ParseNode* n, ParseTree& tree) {
  // (?<!S|T) => (?<!S)(?<!T)
  ParseNode* a = n->Child.Left;
  ParseNode* s = a->Child.Left;
  ParseNode* t = a->Child.Right;

  n->Type = ParseNode::CONCATENATION;
  a->Type = ParseNode::LOOKBEHIND_NEG;
  a->Child.Left = s;
  a->Child.Right = nullptr;
  n->Child.Right = tree.add(ParseNode::LOOKBEHIND_NEG, t);
}

void reduceNegativeLookbehindRepetition(ParseNode* n, ParseTree& tree) {
  // (?<!S{n,m}), (?<!S{n,m}?) => (?<!S...S)
  //                                  \___/
  //                                    n

  // (?<!S{n,m}), (?<!S{n,m}?) => (?<!S{n-1}S)   for n > 2
  // (?<!S{n,m}), (?<!S{n,m}?) => (?<!SS)        for n = 2
  // (?<!S{n,m}), (?<!S{n,m}?) => (?<!S)         for n = 1
  // (?<!S{n,m}), (?<!S{n,m}?) => nothing        for n = 0

  ParseNode* r = n->Child.Left;
  ParseNode* s = r->Child.Left;

  const uint32_t rep = r->Child.Rep.Min;
  switch (rep) {
  case 0:
    n->Child.Left = s;
    n->Type = ParseNode::REPETITION;
    n->Child.Rep.Min = n->Child.Rep.Max = 0;
    break;
  case 1:
    n->Child.Left = s;
    break;
  case 2:
    r->Type = ParseNode::CONCATENATION;
    r->Child.Right = tree.copySubtree(s);
    break;
  default:
    r->Type = ParseNode::CONCATENATION;
    r->Child.Right =
      tree.add(ParseNode::REPETITION, tree.copySubtree(s), rep - 1, rep - 1);
    break;
  }
}

void reduceNegativeLookbehindLookaround(ParseNode* n) {
  // (?<!(?<!S)) => (?<=S)
  // (?<!(?<=S)) => (?<!S)
  // (?<!(?!S))  => (?=S)
  // (?<!(?=S))  => (?!S)

  ParseNode* c = n->Child.Left;
  ParseNode* s = c->Child.Left;

  switch (c->Type) {
  case ParseNode::LOOKAHEAD_POS:
    n->Type = ParseNode::LOOKAHEAD_NEG;
    break;

  case ParseNode::LOOKAHEAD_NEG:
    n->Type = ParseNode::LOOKAHEAD_POS;
    break;

  case ParseNode::LOOKBEHIND_POS:
    n->Type = ParseNode::LOOKBEHIND_NEG;
    break;

  case ParseNode::LOOKBEHIND_NEG:
    n->Type = ParseNode::LOOKBEHIND_POS;
    break;

  default:
    throw std::logic_error("wtf");
  }

  n->Child.Left = s;
}

void reduceNegativeLookaheadLiteral(ParseNode* n, ParseTree& tree) {
  // (?![l]) => (?=[^l])|\Z

  ParseNode* l = n->Child.Left;

  if (l->Type == ParseNode::DOT) {
    // this is a \Z, do nothing
    return;
  }

  n->Type = ParseNode::ALTERNATION;
  n->Child.Right =
    tree.add(ParseNode::LOOKAHEAD_NEG, tree.add(ParseNode::DOT, '.'));

  switch (l->Type) {
  case ParseNode::CHAR_CLASS:
// FIXME: this is probably wrong in the presence of breakout bytes
    l->Set.CodePoints.flip();
    break;

  case ParseNode::LITERAL:
    *l = ParseNode(ParseNode::CHAR_CLASS, ~UnicodeSet(l->Val));
    break;

  case ParseNode::BYTE:
    *l = ParseNode(ParseNode::CHAR_CLASS, UnicodeSet(), ~ByteSet(l->Val), true);
    break;

  default:
    throw std::logic_error("wtf");
  }

  n->Child.Left = tree.add(ParseNode::LOOKAHEAD_POS, l);
}

void reduceNegativeLookaheadConcatenation(ParseNode* n, ParseTree& tree) {
  // (?!ST) => (?!S)|(?=S(?!T))
  ParseNode* c = n->Child.Left;
  ParseNode* s = c->Child.Left;
  ParseNode* t = c->Child.Right;

  n->Type = ParseNode::ALTERNATION;

  c->Type = ParseNode::LOOKAHEAD_NEG;
  c->Child.Left = s;

  n->Child.Right = tree.add(ParseNode::LOOKAHEAD_POS,
    tree.add(ParseNode::CONCATENATION,
      tree.copySubtree(s),
      tree.add(ParseNode::LOOKAHEAD_NEG, t)
    )
  );
}

void reduceNegativeLookaheadAlternation(ParseNode* n, ParseTree& tree) {
  // (?!S|T) => (?!S)(?!T)
  ParseNode* a = n->Child.Left;
  ParseNode* s = a->Child.Left;
  ParseNode* t = a->Child.Right;

  n->Type = ParseNode::CONCATENATION;
  a->Type = ParseNode::LOOKAHEAD_NEG;
  a->Child.Left = s;
  a->Child.Right = nullptr;
  n->Child.Right = tree.add(ParseNode::LOOKAHEAD_NEG, t);
}

void reduceNegativeLookaheadRepetition(ParseNode* n, ParseTree& tree) {
  // (?!S{n,m}), (?!S{n,m}?) => (?!S...S)
  //                               \___/
  //                                 n
  //
  // (?!S{n,m}), (?!S{n,m}?) => (?!S{n-1}S)   for n > 2
  // (?!S{n,m}), (?!S{n,m}?) => (?!SS)        for n = 2
  // (?!S{n,m}), (?!S{n,m}?) => (?!S)         for n = 1
  // (?!S{n,m}), (?!S{n,m}?) => nothing       for n = 0

  ParseNode* r = n->Child.Left;
  ParseNode* s = r->Child.Left;

  const uint32_t rep = r->Child.Rep.Min;
  switch (rep) {
  case 0:
    n->Child.Left = s;
    n->Type = ParseNode::REPETITION;
    n->Child.Rep.Min = n->Child.Rep.Max = 0;
    break;
  case 1:
    n->Child.Left = s;
    break;
  case 2:
    r->Type = ParseNode::CONCATENATION;
    r->Child.Right = tree.copySubtree(s);
    break;
  default:
    r->Type = ParseNode::CONCATENATION;
    r->Child.Right =
      tree.add(ParseNode::REPETITION, tree.copySubtree(s), rep - 1, rep - 1);
    break;
  }
}

void reduceNegativeLookaheadLookaround(ParseNode* n) {
  // (?!(?<!S)) => (?<=S)
  // (?!(?<=S)) => (?<!S)
  // (?!(?!S))  => (?=S)
  // (?!(?=S))  => (?!S)

  ParseNode* c = n->Child.Left;
  ParseNode* s = c->Child.Left;

  switch (c->Type) {
  case ParseNode::LOOKAHEAD_POS:
    n->Type = ParseNode::LOOKAHEAD_NEG;
    break;

  case ParseNode::LOOKAHEAD_NEG:
    n->Type = ParseNode::LOOKAHEAD_POS;
    break;

  case ParseNode::LOOKBEHIND_POS:
    n->Type = ParseNode::LOOKBEHIND_NEG;
    break;

  case ParseNode::LOOKBEHIND_NEG:
    n->Type = ParseNode::LOOKBEHIND_POS;
    break;

  default:
    throw std::logic_error("wtf");
  }

  n->Child.Left = s;
}

bool reduceNegativeLookarounds(ParseNode* n, ParseTree& tree) {
  bool ret = false;

  switch (n->Type) {
  case ParseNode::REGEXP:
    if (!n->Child.Left) {
      return ret;
    }
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKBEHIND_POS:
    ret = reduceNegativeLookarounds(n->Child.Left, tree);
    break;

  case ParseNode::LOOKAHEAD_NEG:
    switch (n->Child.Left->Type) {
    case ParseNode::REPETITION:
    case ParseNode::REPETITION_NG:
      reduceNegativeLookaheadRepetition(n, tree);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::LOOKAHEAD_POS:
    case ParseNode::LOOKAHEAD_NEG:
    case ParseNode::LOOKBEHIND_POS:
    case ParseNode::LOOKBEHIND_NEG:
      reduceNegativeLookaheadLookaround(n);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::ALTERNATION:
      reduceNegativeLookaheadAlternation(n, tree);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::CONCATENATION:
      reduceNegativeLookaheadConcatenation(n, tree);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::DOT:
    case ParseNode::CHAR_CLASS:
    case ParseNode::LITERAL:
    case ParseNode::BYTE:
      reduceNegativeLookaheadLiteral(n, tree);
      break;

    default:
      throw std::logic_error("wtf");
    }
    ret = true;
    break;

  case ParseNode::LOOKBEHIND_NEG:
    switch (n->Child.Left->Type) {
    case ParseNode::REPETITION:
    case ParseNode::REPETITION_NG:
      reduceNegativeLookbehindRepetition(n, tree);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::LOOKAHEAD_POS:
    case ParseNode::LOOKAHEAD_NEG:
    case ParseNode::LOOKBEHIND_POS:
    case ParseNode::LOOKBEHIND_NEG:
      reduceNegativeLookbehindLookaround(n);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::ALTERNATION:
      reduceNegativeLookbehindAlternation(n, tree);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::CONCATENATION:
      reduceNegativeLookbehindConcatenation(n, tree);
      reduceNegativeLookarounds(n, tree);
      break;

    case ParseNode::DOT:
    case ParseNode::CHAR_CLASS:
    case ParseNode::LITERAL:
    case ParseNode::BYTE:
      reduceNegativeLookbehindLiteral(n, tree);
      break;

    default:
      throw std::logic_error("wtf");
    }
    ret = true;
    break;

  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    ret = reduceNegativeLookarounds(n->Child.Left, tree);
    ret |= reduceNegativeLookarounds(n->Child.Right, tree);
    break;

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    break;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }

  return ret;
}

bool reduceNegativeLookarounds(ParseTree& tree) {
  tree.expand(estimateNegativeLookaroundBlowup(tree.Root));
  return reduceNegativeLookarounds(tree.Root, tree);
}

void reduceLookaroundRepetitions(ParseNode* n, std::stack<ParseNode*>& branch) {
  // (?=S){n,m}  = x{0}   if n = 0
  // (?=S){n,m)  = (?=S)  o/w

  // (?<=S){n,m} = x{0}   if n = 0
  // (?<=S){n,m) = (?<=S) o/w

  if (n->Child.Rep.Min == 0) {
    n->Type = ParseNode::REPETITION;
    n->Child.Rep.Max = 0;
    n->Child.Left->Type = ParseNode::LITERAL;
    n->Child.Left->Val = 'x';
  }
  else {
    spliceOutParent(branch.top(), n, n->Child.Left);
  }
}

void reduceNestedLookarounds(ParseNode* n, std::stack<ParseNode*>& branch) {
  // (?=(?=S))  = (?<=(?=S)   = (?=S)
  // (?=(?<=S)) = (?<=(?<=S)) = (?<=S)
  // (?=\A)     = (?<=\A)     = \A
  // (?=\Z)     = (?<=\Z)     = \Z
  spliceOutParent(branch.top(), n, n->Child.Left);
}

bool matchesAtEnd(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return !n->Child.Left || matchesAtEnd(n->Child.Left);

  case ParseNode::LOOKAHEAD_POS:
    return matchesAtEnd(n->Child.Left);

  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_NEG:
    return true;

  case ParseNode::ALTERNATION:
    return matchesAtEnd(n->Child.Left) || matchesAtEnd(n->Child.Right);

  case ParseNode::CONCATENATION:
    return matchesAtEnd(n->Child.Left) && matchesAtEnd(n->Child.Right);

  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    return n->Child.Rep.Min == 0 || matchesAtEnd(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return false;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

bool matchesBeforeStart(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return !n->Child.Left || matchesBeforeStart(n->Child.Left);

  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
  case ParseNode::LOOKBEHIND_NEG:
    return true;

  case ParseNode::LOOKBEHIND_POS:
    return matchesBeforeStart(n->Child.Left);

  case ParseNode::ALTERNATION:
    return matchesBeforeStart(n->Child.Left) ||
           matchesBeforeStart(n->Child.Right);

  case ParseNode::CONCATENATION:
    return matchesBeforeStart(n->Child.Left) &&
           matchesBeforeStart(n->Child.Right);

  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    return n->Child.Rep.Min == 0 || matchesBeforeStart(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return false;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

bool shoveAnchorsOutward(ParseNode* p, ParseNode* n) {
  bool ret = false;

  if (n->Type == ParseNode::CONCATENATION) {
    if (n->Child.Left->Type == ParseNode::LOOKAHEAD_NEG) {
      /*
          Where X is any internal node:

             X         X                        X
             |         |                        |
             &    =>  ?!  if T matches    [^\z00-\zFF]  otherwise
            / \        |  at end,
           ?!  T       .
           |
           .
      */
      if (matchesAtEnd(n->Child.Right)) {
        spliceOutParent(p, n, n->Child.Left);
      }
      else {
        *n = ParseNode(ParseNode::CHAR_CLASS, ByteSet());
      }
      ret = true;
    }
    else if (n->Child.Right->Type == ParseNode::LOOKBEHIND_NEG) {
      /*
          Where X is any internal node:

             X         X                        X
             |         |                        |
             &    =>  ?<!  if T matches    [^\z00-\zFF]  otherwise
            / \        |   before start,
           T ?<!       .
              |
              .
      */
      if (matchesBeforeStart(n->Child.Left)) {
        spliceOutParent(p, n, n->Child.Right);
      }
      else {
        *n = ParseNode(ParseNode::CHAR_CLASS, ByteSet());
      }
      ret = true;
    }
  }

  return ret;
}

void distributeDisjunctionOverPositiveLookarounds(ParseNode* n) {
  ParseNode* l = n->Child.Left;
  ParseNode* r = n->Child.Right;
  ParseNode* s = l->Child.Left;
  ParseNode* t = r->Child.Left;

  n->Type = l->Type;
  n->Child.Right = nullptr;

  l->Type = ParseNode::ALTERNATION;
  l->Child.Left = s;
  l->Child.Right = t;
}

bool isLiteral(ParseNode* n) {
  switch (n->Type) {
  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return true;
  default:
    return false;
  }
}

void literalToCC(ParseNode* n) {
  switch (n->Type) {
  case ParseNode::DOT:
    *n = ParseNode(ParseNode::CHAR_CLASS, ~UnicodeSet());
    break;

  case ParseNode::CHAR_CLASS:
    // do nothing
    break;

  case ParseNode::LITERAL:
    *n = ParseNode(ParseNode::CHAR_CLASS, UnicodeSet(n->Val));
    break;

  case ParseNode::BYTE:
    *n = ParseNode(ParseNode::CHAR_CLASS, ByteSet(n->Val));
    break;

  default:
    throw std::logic_error("wtf");
  }
}

void makeParentMap(ParseNode* n, std::map<ParseNode*, ParseNode*>& parent) {
  switch (n->Type) {
  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    parent[n->Child.Right] = n;
    makeParentMap(n->Child.Right, parent);

  case ParseNode::REGEXP:
  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    parent[n->Child.Left] = n;
    makeParentMap(n->Child.Left, parent);
    break;

  default:
    break;
  }
}

void restartShove(ParseNode* root, std::stack<ParseNode*>& check) {
  while (!check.empty()) {
    check.pop();
  }
  check.push(root);
}

bool shoveLookaroundsOutward(ParseTree& tree) {
  bool ret = false;
  ParseNode* root = tree.Root;

  std::map<ParseNode*, ParseNode*> parent;
  makeParentMap(root, parent);

  std::stack<ParseNode*> check;
  check.push(root);

  ParseNode* n;
  while (!check.empty()) {
    n = check.top();
    check.pop();

    switch (n->Type) {
    case ParseNode::REGEXP:
      check.push(n->Child.Left);
      break;

    case ParseNode::LOOKBEHIND_POS:
    case ParseNode::LOOKBEHIND_NEG:
    case ParseNode::LOOKAHEAD_POS:
    case ParseNode::LOOKAHEAD_NEG:
      {
        check.push(n->Child.Left);
        ParseNode* p = parent[n];

        // reduce repetitions of lookarounds
        if (p->Type == ParseNode::REPETITION ||
            p->Type == ParseNode::REPETITION_NG) {
          /*
             (?=S){n,m} = (?<=S){n,m} = \A = \Z = x{0}   if n = 0

             (?=S){n,m)  = (?=S)
             (?<=S){n,m} = (?<=S)   o/w
             \A{n,m}     = \A
             \Z{n,m}     = \Z
          */
          ParseNode* gp = parent[p];
          if (p->Child.Rep.Min == 0) {
            p->Child.Rep.Max = 0;
            p->Type = ParseNode::REPETITION;
            *n = ParseNode(ParseNode::LITERAL, 'x');
          }
          else {
            spliceOutParent(gp, p, n);
            p->Type = ParseNode::TEMPORARY;
            parent[n] = gp;
          }

          restartShove(root, check);
          ret = true;
          break;
        }

        if (n->Type == ParseNode::LOOKAHEAD_NEG &&
            p->Type == ParseNode::CONCATENATION && n == p->Child.Left) {
          /*
              Where X is any internal node:

                 X         X                        X
                 |         |                        |
                 &    =>  ?!  if T matches    [^\z00-\zFF]  otherwise
                / \        |  at end,
               ?!  T       .
               |
               .
          */
          ParseNode* gp = parent[p];
          if (matchesAtEnd(p->Child.Right)) {
            spliceOutParent(gp, p, n);
            p->Type = ParseNode::TEMPORARY;
            parent[n] = gp;
          }
          else {
            n->Type = p->Child.Right->Type = ParseNode::TEMPORARY;
            *p = ParseNode(ParseNode::CHAR_CLASS, ByteSet());
          }
          restartShove(root, check);
          ret = true;
          break;
        }

        if (n->Type == ParseNode::LOOKBEHIND_NEG &&
            p->Type == ParseNode::CONCATENATION && n == p->Child.Right) {
          /*
              Where X is any internal node:

                 X         X                        X
                 |         |                        |
                 &    =>  ?<!  if T matches    [^\z00-\zFF]  otherwise
                / \        |   before start,
               T ?<!       .
                  |
                  .
          */
          ParseNode* gp = parent[p];
          if (matchesBeforeStart(p->Child.Left)) {
            spliceOutParent(gp, p, n);
            p->Type = ParseNode::TEMPORARY;
            parent[n] = gp;
          }
          else {
            n->Type = p->Child.Left->Type = ParseNode::TEMPORARY;
            *p = ParseNode(ParseNode::CHAR_CLASS, ByteSet());
          }
          restartShove(root, check);
          ret = true;
          break;
        }

        if (p->Type == ParseNode::LOOKAHEAD_POS ||
            p->Type == ParseNode::LOOKBEHIND_POS) {
          // (?=(?=S))  = (?<=(?=S)   = (?=S)
          // (?=(?<=S)) = (?<=(?<=S)) = (?<=S)
          // (?=\A)     = (?<=\A)     = \A
          // (?=\Z)     = (?<=\Z)     = \Z
          ParseNode* gp = parent[p];
          spliceOutParent(gp, p, n);
          p->Type = ParseNode::TEMPORARY;
          parent[n] = gp;
          restartShove(root, check);
          ret = true;
          break;
        }

        if (n->Type == ParseNode::LOOKBEHIND_POS &&
            n->Child.Left->Type == ParseNode::CONCATENATION &&
            n->Child.Left->Child.Left->Type == ParseNode::LOOKBEHIND_POS)
        {
          /*
                 ?<=         ?<=
                  |           |
                  &     =>    &
                 / \         / \
               ?<=  T       S   T
                |
                S
          */
          ParseNode* c = n->Child.Left;
          ParseNode* l = c->Child.Left;
          ParseNode* s = l->Child.Left;

          spliceOutParent(c, l, s);

          parent[s] = c;
          l->Type = ParseNode::TEMPORARY;

          restartShove(root, check);
          ret = true;
          break;
        }

        if (n->Type == ParseNode::LOOKAHEAD_POS &&
            n->Child.Left->Type == ParseNode::CONCATENATION) {
          p = n;
          ParseNode* c = n->Child.Left;
          while (c->Type == ParseNode::CONCATENATION) {
            p = c;
            c = c->Child.Right;
          }

          if (c->Type == ParseNode::LOOKAHEAD_POS) {
            /*
                  ?=                ?=
                   |                 |
                   &                 &
                  / \               / \
                 T0  &       =>    T0  &
                    / \               / \
                   T1 ...            T1 ...
                       &                 &
                      / \               / \
                     Tk ?=             Tk  S
                         |
                         S
            */
            spliceOutParent(p, c, c->Child.Left);
            parent[c->Child.Left] = p;
            c->Type = ParseNode::TEMPORARY;
            restartShove(root, check);
            ret = true;
            break;
          }
        }
      }
      break;

    case ParseNode::ALTERNATION:
      check.push(n->Child.Left);
      check.push(n->Child.Right);

      if ((n->Child.Left->Type  == ParseNode::LOOKAHEAD_POS &&
           n->Child.Right->Type == ParseNode::LOOKAHEAD_POS) ||
          (n->Child.Left->Type  == ParseNode::LOOKBEHIND_POS &&
           n->Child.Right->Type == ParseNode::LOOKBEHIND_POS)) {
        /*
              |         ?=              |          ?<=
             / \         |             / \          |
           ?=  ?=   =>   |           ?<= ?<=   =>   |
            |   |       / \           |   |        / \
            S   T      S   T          S   T       S   T
        */
        ParseNode* l = n->Child.Left;
        ParseNode* r = n->Child.Right;
        ParseNode* t = r->Child.Left;

        n->Type = l->Type;
        n->Child.Right = nullptr;

        l->Type = ParseNode::ALTERNATION;
        l->Child.Right = t;
        parent[t] = l;

        r->Type = ParseNode::TEMPORARY;

        restartShove(root, check);
        ret = true;
      }
      break;

    case ParseNode::CONCATENATION:
      check.push(n->Child.Left);
      check.push(n->Child.Right);

      if ((n->Child.Left->Type == ParseNode::LOOKAHEAD_POS ||
           n->Child.Left->Type == ParseNode::LOOKAHEAD_NEG)) {
        /*
           Lookarounds commute. Reorder opposite-facing lookarounds.

           (?=S)(?<=T) = (?<=T)(?=S)
           \Z(?<=T)    = (?<=T)\Z
           (?=S)\A     = \A(?=S)
        */
        if (n->Child.Right->Type == ParseNode::LOOKBEHIND_POS ||
           n->Child.Right->Type == ParseNode::LOOKBEHIND_NEG) {
          /*
                &          &
               / \   =>   / \
              LA LB      LB LA
          */
          std::swap(n->Child.Left, n->Child.Right);
          restartShove(root, check);
          ret = true;
          break;
        }
        else if (n->Child.Right->Type == ParseNode::CONCATENATION &&
            (n->Child.Right->Child.Left->Type == ParseNode::LOOKBEHIND_POS ||
             n->Child.Right->Child.Left->Type == ParseNode::LOOKBEHIND_NEG)) {
          /*
                &          &
               / \   =>   / \
              LA  &      LB  &
                 / \        / \
                LB  X      LA  X
          */
          std::swap(n->Child.Left, n->Child.Right->Child.Left);
          parent[n->Child.Left] = n;
          parent[n->Child.Right->Child.Left] = n->Child.Right;
          restartShove(root, check);
          ret = true;
          break;
        }
      }

      if (n->Child.Left->Type == ParseNode::LOOKAHEAD_POS) {
        /*
             &          &
            / \        / \
           ?=  T  =>  T' ?=
            |             |
            S             S'
        */

        if (n->Child.Right->Type == ParseNode::ALTERNATION) {
          /*
            Apply De Morgan's Law to distribute lookaheads righwards
            over alternations:

                &               |
               / \            /   \
             ?=   |    =>    &     &
             |   / \        / \   / \
             S  T   U      ?=  T ?=  U
                           |     |
                           S     S

            This case is inflationary.
          */

          ParseNode* l = n->Child.Left;
          ParseNode* r = n->Child.Right;
          ParseNode* t = r->Child.Left;

          n->Type = ParseNode::ALTERNATION;

          ParseNode* l_t = tree.add(ParseNode::CONCATENATION, l, t);
          n->Child.Left = l_t;
          parent[l_t] = n;
          parent[l] = parent[t] = l_t;

          ParseNode* ldup = tree.copySubtree(l);
          makeParentMap(ldup, parent);

          r->Type = ParseNode::CONCATENATION;
          r->Child.Left = ldup;
          parent[ldup] = r;

          restartShove(root, check);
          ret = true;
          break;
        }

        if (isLiteral(n->Child.Left->Child.Left) &&
            isLiteral(n->Child.Right))
        {
          /*
                &            &
               / \         /   \
              ?=  b  =>  {0} [a&&b]
               |          |
               a          a
          */

          ParseNode* l = n->Child.Left;
          ParseNode* a = l->Child.Left;
          ParseNode* b = n->Child.Right;

          literalToCC(a);
          literalToCC(b);

          b->Set.CodePoints &= a->Set.CodePoints;
  // FIXME: handle breakout bytes properly
          b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

          l->Type = ParseNode::REPETITION;
          l->Child.Rep.Min = l->Child.Rep.Max = 0;

          restartShove(root, check);
          ret = true;
          break;
        }

        if (isLiteral(n->Child.Left->Child.Left) &&
            n->Child.Right->Type == ParseNode::CONCATENATION &&
            isLiteral(n->Child.Right->Child.Left))
        {
          /*
                  &               &
                 / \            /   \
               ?=   &    =>  [a&&b]  X
               |   / \
               a  b   X
          */

          ParseNode* l = n->Child.Left;
          ParseNode* a = l->Child.Left;
          ParseNode* r = n->Child.Right;
          ParseNode* b = r->Child.Left;
          ParseNode* x = r->Child.Right;

          literalToCC(a);
          literalToCC(b);

          b->Set.CodePoints &= a->Set.CodePoints;
  // FIXME: handle breakout bytes properly
          b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

          n->Child.Left = b;
          n->Child.Right = x;

          l->Type = r->Type = a->Type = ParseNode::TEMPORARY;
          parent[b] = parent[x] = n;

          restartShove(root, check);
          ret = true;
          break;
        }

        if (n->Child.Left->Child.Left->Type == ParseNode::CONCATENATION &&
            isLiteral(n->Child.Left->Child.Left->Child.Left) &&
            isLiteral(n->Child.Right))
        {
          /*
                  &             &
                 / \          /   \
               ?=   b  =>  [a&&b] ?=
                |                  |
                &                  S
               / \
              a   S
          */
          ParseNode* l = n->Child.Left;
          ParseNode* ll = l->Child.Left;
          ParseNode* a = ll->Child.Left;
          ParseNode* s = ll->Child.Right;
          ParseNode* b = n->Child.Right;

          literalToCC(a);
          literalToCC(b);

          b->Set.CodePoints &= a->Set.CodePoints;
  // FIXME: handle breakout bytes properly
          b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

          n->Child.Left = b;
          n->Child.Right = l;

          l->Child.Left = s;

          ll->Type = a->Type = ParseNode::TEMPORARY;
          parent[b] = parent[l] = n;
          parent[s] = l;

          restartShove(root, check);
          ret = true;
          break;
        }

        if (n->Child.Left->Child.Left->Type == ParseNode::CONCATENATION &&
            isLiteral(n->Child.Left->Child.Left->Child.Left) &&
            n->Child.Right->Type == ParseNode::CONCATENATION &&
            isLiteral(n->Child.Right->Child.Left))
        {
           /*
                  &               &
                 / \            /   \
               ?=   &    =>  [a&&b]  &
               |   / \              / \
               &  b   T           ?=   T
              / \                  |
             a   S                 S
          */

          ParseNode* l = n->Child.Left;
          ParseNode* ll = l->Child.Left;
          ParseNode* a = ll->Child.Left;
          ParseNode* s = ll->Child.Right;
          ParseNode* r = n->Child.Right;
          ParseNode* b = r->Child.Left;

          literalToCC(a);
          literalToCC(b);

          b->Set.CodePoints &= a->Set.CodePoints;
  // FIXME: handle breakout bytes properly
          b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

          n->Child.Left = b;
          r->Child.Left = l;
          l->Child.Left = s;

          ll->Type = a->Type = ParseNode::TEMPORARY;
          parent[b] = n;
          parent[l] = r;
          parent[s] = l;

          restartShove(root, check);
          ret = true;
          break;
        }
      }

      if (n->Child.Right->Type == ParseNode::LOOKBEHIND_POS) {
        /*
             &            &
            / \          / \
           T ?<=   =>  ?<=  T'
               |        |
               S        S'
        */

        if (n->Child.Left->Type == ParseNode::ALTERNATION) {
          /*
            Apply De Morgan's Law to distribute lookbehinds leftwards
            over alternations:

                &               |
               / \            /   \
              |   ?<=  =>    &     &
             / \    |       / \   / \
            T   U   S      T ?<= U ?<=
                              |     |
                              S     S

            This case is inflationary.
          */

          ParseNode* l = n->Child.Left;
          ParseNode* r = n->Child.Right;
          ParseNode* u = l->Child.Right;

          n->Type = ParseNode::ALTERNATION;

          l->Type = ParseNode::CONCATENATION;
          l->Child.Right = r;
          parent[r] = l;

          ParseNode* rdup = tree.copySubtree(r);
          makeParentMap(rdup, parent);

          ParseNode* u_rdup = tree.add(ParseNode::CONCATENATION, u, rdup);
          parent[u] = parent[rdup] = u_rdup;
          n->Child.Right = u_rdup;
          parent[u_rdup] = n;

          restartShove(root, check);
          ret = true;
          break;
        }

        if (isLiteral(n->Child.Right->Child.Left) &&
            isLiteral(n->Child.Left))
        {
          /*
                &            &
               / \         /   \
              b ?<=  => [a&&b] {0}
                  |             |
                  a             a
          */

          ParseNode* r = n->Child.Right;
          ParseNode* a = r->Child.Left;
          ParseNode* b = n->Child.Left;

          literalToCC(a);
          literalToCC(b);

          b->Set.CodePoints &= a->Set.CodePoints;
    // FIXME: handle breakout bytes properly
          b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

          r->Type = ParseNode::REPETITION;
          r->Child.Rep.Min = r->Child.Rep.Max = 0;

          restartShove(root, check);
          ret = true;
          break;
        }

        if (n->Child.Right->Child.Left->Type == ParseNode::CONCATENATION){
          /*
                &                 &
               / \              /   \
              b  ?<=    =>    ?<=  [a&&b]
                  |            |
                  &            &
                 / \          / \
                T0  &        T0  &
                   / \          / \
                  T1 ...      T1 ...
                      &           &
                     / \         / \
                    Tk  a     Tk-1  Tk
          */

          ParseNode* b = n->Child.Left;
          ParseNode* r = n->Child.Right;

          ParseNode* p = r;
          ParseNode* a = p->Child.Left;
          while (a->Type == ParseNode::CONCATENATION) {
            p = a;
            a = a->Child.Right;
          }

          if (isLiteral(a)) {
            literalToCC(a);
            literalToCC(b);

            b->Set.CodePoints &= a->Set.CodePoints;
            // FIXME: handle breakout bytes properly
            b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

            n->Child.Left = r;
            n->Child.Right = b;

            ParseNode* gp = parent[p];
            ParseNode* tk = p->Child.Left;
            spliceOutParent(gp, p, tk);
            parent[tk] = gp;

            a->Type = ParseNode::TEMPORARY;

            restartShove(root, check);
            ret = true;
            break;
          }
        }
      }

      if (n->Child.Right->Type == ParseNode::CONCATENATION &&
          n->Child.Right->Child.Left->Type == ParseNode::LOOKBEHIND_POS) {
        if (isLiteral(n->Child.Left)) {
          if (isLiteral(n->Child.Right->Child.Left->Child.Left)) {
            /*
                  &               &
                 / \            /   \
                b   &    =>  [a&&b]  S
                   / \
                 ?<=  S
                  |
                  a
            */

            ParseNode* b = n->Child.Left;
            ParseNode* r = n->Child.Right;
            ParseNode* rl = r->Child.Left;
            ParseNode* a = rl->Child.Left;
            ParseNode* s = r->Child.Right;

            literalToCC(a);
            literalToCC(b);

            b->Set.CodePoints &= a->Set.CodePoints;
        // FIXME: handle breakout bytes properly
            b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

            n->Child.Right = s;
            parent[s] = n;

            r->Type = rl->Type = a->Type = ParseNode::TEMPORARY;

            restartShove(root, check);
            ret = true;
            break;
          }

          if (n->Child.Right->Child.Left->Child.Left->Type == ParseNode::CONCATENATION) {
            /*
                  &                 &
                 / \              /   \
                b   &    =>    ?<=     &
                   / \          |     /  \
                 ?<=  S         &  [a&&b] S
                  |            / \
                  &           T0  &
                 / \             / \
                T0  &           T1 ...
                   / \              &
                  T1 ...           / \
                      &         Tk-1  Tk
                     / \
                    Tk  a
            */

            ParseNode* b = n->Child.Left;
            ParseNode* r = n->Child.Right;
            ParseNode* rl = r->Child.Left;

            ParseNode* p = rl;
            ParseNode* a = p->Child.Left;
            while (a->Type == ParseNode::CONCATENATION) {
              p = a;
              a = a->Child.Right;
            }

            if (isLiteral(a)) {
              literalToCC(a);
              literalToCC(b);

              b->Set.CodePoints &= a->Set.CodePoints;
              // FIXME: handle breakout bytes properly
              b->Set.Breakout.Bytes &= a->Set.Breakout.Bytes;

              n->Child.Left = rl;
              parent[rl] = n;
              r->Child.Left = b;
              parent[b] = r;

              ParseNode* gp = parent[p];
              ParseNode* tk = p->Child.Left;
              spliceOutParent(gp, p, tk);
              parent[tk] = gp;

              a->Type = ParseNode::TEMPORARY;

              restartShove(root, check);
              ret = true;
              break;
            }
          }
        }
      }
      break;

    case ParseNode::REPETITION:
    case ParseNode::REPETITION_NG:
      check.push(n->Child.Left);

      // reduce empty repetitions
      if (n->Child.Rep.Max == 0) {
        ParseNode* p = parent[n];

        switch (p->Type) {
        case ParseNode::REGEXP:
          break;

        case ParseNode::LOOKBEHIND_POS:
        case ParseNode::LOOKAHEAD_POS:
        case ParseNode::REPETITION:
        case ParseNode::REPETITION_NG:
          {
            ParseNode* gp = parent[p];
            spliceOutParent(gp, p, n);
            p->Type = ParseNode::TEMPORARY;
            parent[n] = gp;
            restartShove(root, check);
            ret = true;
          }
          break;

        case ParseNode::CONCATENATION:
          {
            /*
                  &       &
                 / \     / \
               {0}  T   T  {0}  =>  T
                |           |
                x           x
            */
            ParseNode* gp = parent[p];
            ParseNode* s = n == p->Child.Left ? p->Child.Right : p->Child.Left;
            spliceOutParent(gp, p, s);
            n->Type = p->Type = ParseNode::TEMPORARY;
            parent[s] = gp;
            restartShove(root, check);
            ret = true;
          }
          break;

        case ParseNode::ALTERNATION:
          {
            /*
                  |                  |
                 / \       ??       / \        ?
               {0}  T  =>   |      T  {0}  =>  |
                |           T      |           T
                x                  x
            */
            ParseNode* t = n == p->Child.Left ? p->Child.Right : p->Child.Left;
            p->Type = n == p->Child.Left ? ParseNode::REPETITION_NG : ParseNode::REPETITION;

            p->Child.Rep.Min = 0;
            p->Child.Rep.Max = 1;
            p->Child.Left = t;
            p->Child.Right = nullptr;

            n->Type = n->Child.Left->Type = ParseNode::TEMPORARY;
            restartShove(root, check);
            ret = true;
          }
          break;

        default:
          throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
        }
      }
      break;

    case ParseNode::CHAR_CLASS:
      if (n->Set.CodePoints.none() && n->Set.Breakout.Bytes.none()) {
        // impossible atom, kill this branch
        ParseNode* p = parent[n];
        switch (p->Type) {
        case ParseNode::REGEXP:
          break;

        case ParseNode::LOOKBEHIND_POS:
        case ParseNode::LOOKAHEAD_POS:
          {
            /*
                 ?<=   ?=
                   |    |  =>  []
                  []   []

            */
            ParseNode* gp = parent[p];
            spliceOutParent(gp, p, n);
            p->Type = ParseNode::TEMPORARY;
            parent[n] = gp;
            restartShove(root, check);
            ret = true;
          }
          break;

        case ParseNode::REPETITION:
        case ParseNode::REPETITION_NG:
          {
            /*
                 {n,m} {n,m}?      []  if n > 0
                   |     |     =>
                  []    []         {0} o/w
                                    |
                                   []
            */
            if (p->Child.Rep.Min == 0) {
              p->Child.Rep.Max = 0;
            }
            else {
              ParseNode* gp = parent[p];
              spliceOutParent(gp, p, n);
              p->Type = ParseNode::TEMPORARY;
              parent[n] = gp;
            }

            restartShove(root, check);
            ret = true;
          }
          break;

        case ParseNode::CONCATENATION:
          {
            /*
                  &      &
                 / \    / \   => []
                []  T  T  []

            */
            ParseNode* gp = parent[p];
            spliceOutParent(gp, p, n);
            p->Type = ParseNode::TEMPORARY;
            parent[n] = gp;
            restartShove(root, check);
            ret = true;
          }
          break;

        case ParseNode::ALTERNATION:
          {
            /*
                  |      |
                 / \    / \   => T
                []  T  T  []

            */
            ParseNode* gp = parent[p];
            ParseNode* t = n == p->Child.Left ? p->Child.Right : p->Child.Left;
            spliceOutParent(gp, p, t);
            p->Type = n->Type = ParseNode::TEMPORARY;
            parent[t] = gp;
            restartShove(root, check);
            ret = true;
          }
          break;

        default:
          break;
        }
      }
      break;

    case ParseNode::DOT:
    case ParseNode::LITERAL:
    case ParseNode::BYTE:
    default:
      break;
    }
  }

  return ret;
}

// TODO: combine concatenations of same-type lookarounds

size_t blowupTreeSize(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return 1 + (!n->Child.Left ? 0 : blowupTreeSize(n->Child.Left));

  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_NEG:
    return 1 + blowupTreeSize(n->Child.Left);

  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    return 1 + blowupTreeSize(n->Child.Left) + blowupTreeSize(n->Child.Right);

  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    return n->Child.Rep.Min * blowupTreeSize(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return 1;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

size_t estimateNegativeLookaroundBlowup(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return !n->Child.Left ? 0 : estimateNegativeLookaroundBlowup(n->Child.Left);

  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_NEG:
    return 9*blowupTreeSize(n->Child.Left);

  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    return estimateNegativeLookaroundBlowup(n->Child.Left) +
           estimateNegativeLookaroundBlowup(n->Child.Right);

  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    return estimateNegativeLookaroundBlowup(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return 0;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

std::tuple<ParseNode*,ParseNode*,ParseNode*> splitLookarounds(ParseNode* root) {

  // split tree into lookbehinds, pattern, lookaheads

  ParseNode* behind = nullptr;
  ParseNode* middle = nullptr;
  ParseNode* ahead = nullptr;

  if (root->Child.Left) {
    if (root->Child.Left->Type == ParseNode::CONCATENATION) {
      // check for lookbehinds
      switch (root->Child.Left->Child.Left->Type) {
      case ParseNode::LOOKBEHIND_POS:
      case ParseNode::LOOKBEHIND_NEG:
        /*
              &           &
             / \   OR    / \
           ?<=         ?<!
        */
        behind = root->Child.Left->Child.Left;
        break;

      case ParseNode::ALTERNATION:
        /*
                &           &
               / \   OR    / \
              |           |
             / \         / \
           ?<= ?<!     ?<! ?<=
        */
        {
          ParseNode* alt = root->Child.Left->Child.Left;
          if ((alt->Child.Left->Type == ParseNode::LOOKBEHIND_POS &&
               alt->Child.Right->Type == ParseNode::LOOKBEHIND_NEG) ||
              (alt->Child.Left->Type == ParseNode::LOOKBEHIND_NEG &&
               alt->Child.Right->Type == ParseNode::LOOKBEHIND_POS))
          {
            behind = alt;
          }
        }
        break;

      default:
        break;
      }

      // check for lookaheads
      ParseNode* gp = root;
      ParseNode* p = root->Child.Left;
      ParseNode* a = p->Child.Right;
      while (a->Type == ParseNode::CONCATENATION) {
        gp = p;
        p = a;
        a = a->Child.Right;
      }

      switch (a->Type) {
      case ParseNode::LOOKAHEAD_POS:
      case ParseNode::LOOKAHEAD_NEG:
        /*
              &              &
             / \            / \
                &              &
               / \     OR     / \
                 ...            ...
                  &              &
                 / \            / \
                   ?=             ?!
        */
        ahead = a;
        break;

      case ParseNode::ALTERNATION:
        /*
              &              &
             / \            / \
                &              &
               / \     OR     / \
                 ...            ...
                  &              &
                 / \            / \
                    |              |
                   / \            / \
                  ?= ?!          ?! ?=
        */
        {
          if ((a->Child.Left->Type == ParseNode::LOOKAHEAD_POS &&
               a->Child.Right->Type == ParseNode::LOOKAHEAD_NEG) ||
              (a->Child.Left->Type == ParseNode::LOOKAHEAD_NEG &&
               a->Child.Right->Type == ParseNode::LOOKAHEAD_POS))
          {
            ahead = a;
          }
        }
        break;

      default:
        break;
      }

      // check for the middle
      if (behind) {
        if (!ahead) {
          /*
               &
              / \
             LB  M
          */
          middle = root->Child.Left->Child.Right;
        }
        else {
          /*
              &
             / \
            LB  &
               / \
              M0 ...
                  &
                 / \
                Mk  LA
          */
          middle = root->Child.Left->Child.Right->Child.Left;
          // detach ahead from middle
          spliceOutParent(gp, p, p->Child.Left);
        }
      }
      else if (ahead) {
        /*
              &
             / \
            M  LA
        */
        middle = root->Child.Left->Child.Left;
      }
    }
    else {
      // main operator is not &, therefore we assume no lookarounds
      middle = root->Child.Left;
    }
  }

  return std::tie(behind, middle, ahead);
}

std::tuple<ParseTree, ParseTree, ParseTree> splitLookarounds(const ParseTree& tree) {
  ParseNode* behind;
  ParseNode* middle;
  ParseNode* ahead;
  std::tie(behind, middle, ahead) = splitLookarounds(tree.Root);
  return std::make_tuple(
    ParseTree(behind), ParseTree(middle), ParseTree(ahead)
  );
}

bool containsLookaroundAssertion(const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    return !n->Child.Left ? false : containsLookaroundAssertion(n->Child.Left);

  case ParseNode::LOOKBEHIND_POS:
  case ParseNode::LOOKBEHIND_NEG:
  case ParseNode::LOOKAHEAD_POS:
  case ParseNode::LOOKAHEAD_NEG:
    return true;

  case ParseNode::ALTERNATION:
  case ParseNode::CONCATENATION:
    return containsLookaroundAssertion(n->Child.Left) ||
           containsLookaroundAssertion(n->Child.Right);

  case ParseNode::REPETITION:
  case ParseNode::REPETITION_NG:
    return containsLookaroundAssertion(n->Child.Left);

  case ParseNode::DOT:
  case ParseNode::CHAR_CLASS:
  case ParseNode::LITERAL:
  case ParseNode::BYTE:
    return false;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}
