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

#include "nfabuilder.h"
#include "parsetree.h"
#include "states.h"
#include "transitionfactory.h"
#include "utility.h"
#include "encoders/ascii.h"

#include <algorithm>
#include <iostream>
#include <utility>
#include <cctype>


// static std::ostream& operator<<(std::ostream& out, const InListT& list) {
//   out << '[';
//   for (InListT::const_iterator it(list.begin()); it != list.end(); ++it) {
//     if (it != list.begin()) {
//       out << ", ";
//     }
//     out << *it;
//   }
//   out << ']';
//   return out;
// }

// static std::ostream& operator<<(std::ostream& out, const OutListT& list) {
//   out << '[';
//   for (OutListT::const_iterator i(list.begin()); i != list.end(); ++i) {
//     if (i != list.begin()) {
//       out << ", ";
//     }

//     out << '(' << i->first << ',' << i->second << ')';
//   }
//   out << ']';
//   return out;
// }

// static std::ostream& operator<<(std::ostream& out, const Fragment& f) {
//   out << "in " << f.InList << ", out " << f.OutList
//       << ", skip " << f.Skippable;
//   return out;
// }

NFABuilder::NFABuilder():
  CurLabel(0),
  ReserveSize(0),
  Fsm(new NFA(1)),
  TransFac(new TransitionFactory())
{
  setEncoder(std::shared_ptr<Encoder>(new ASCII));
  init();
}

void NFABuilder::reset() {
  if (Fsm) {
    Fsm->clear();
    Fsm->addVertex();
  }
  else {
    Fsm.reset(new NFA(1));
//    Fsm.reset(new NFA(1, ReserveSize));
  }

  while (!Stack.empty()) {
    Stack.pop();
  }

  init();
}

void NFABuilder::init() {
  IsGood = false;
  TempFrag.initFull(0, ParseNode());
  Stack.push(TempFrag);
  Fsm->TransFac = TransFac;
}

void NFABuilder::setEncoder(const std::shared_ptr<Encoder>& e) {
  Enc = e;
  TempBuf.reset(new byte[Enc->maxByteLength()]);
}

void NFABuilder::setSizeHint(uint64_t reserveSize) {
  ReserveSize = reserveSize;
}

void NFABuilder::patch_mid(OutListT& src, const InListT& dst, uint32_t dstskip) {
  // Make an edge from each vertex in src to each vertex in dst. Edges
  // to vertices in dst before dstskip go before the insertion point in
  // src, edges to vertices in dst after dstskip go after the insertion
  // point in src.
  const InListT::const_iterator skip_stop(
    dstskip < dst.size() ? dst.begin() + dstskip : dst.end());

  for (OutListT::iterator oi(src.begin()); oi != src.end(); ++oi) {
    uint32_t pos = oi->second;

    InListT::const_iterator ii(dst.begin());

    // make edges before dstskip, inserting before src insertion point
    for ( ; ii != dst.end() && ii < skip_stop; ++ii) {
      Fsm->insertEdge(oi->first, *ii, pos++);
    }

    // save the new insertion point for dst
    const uint32_t spos = pos;

    // make edges after dstskip, inserting after src insertion point
    for ( ; ii != dst.end(); ++ii) {
      Fsm->insertEdge(oi->first, *ii, pos++);
    }

    // set the new insertion point for dst
    oi->second = spos;
  }
}

void NFABuilder::patch_pre(OutListT& src, const InListT& dst) {
  // Put all new edges before dst's insertion point.
  patch_mid(src, dst, dst.size());
}

void NFABuilder::patch_post(OutListT& src, const InListT& dst) {
  // Put all new edges after dst's insertion point.
  patch_mid(src, dst, 0);
}

void NFABuilder::literal(const ParseNode& n) {
  const uint32_t len = Enc->write(n.Val, TempBuf.get());
  if (len == 0) {
    THROW_RUNTIME_ERROR_WITH_CLEAN_OUTPUT(
      "code point U+" << std::hex << std::uppercase << n.Val << std::dec
                      << " does not exist in " << Enc->name()
    );
  }

  NFA& g(*Fsm);
  NFA::VertexDescriptor first, prev, last;
  first = prev = last = g.addVertex();
  g[first].Trans = g.TransFac->getByte(TempBuf[0]);
  for (uint32_t i = 1; i < len; ++i) {
    last = g.addVertex();
    g.addEdge(prev, last);
    g[last].Trans = g.TransFac->getByte(TempBuf[i]);
    prev = last;
  }
  TempFrag.reset(n);
  TempFrag.InList.push_back(first);
  TempFrag.OutList.emplace_back(last, 0);
  Stack.push(TempFrag);
}

void NFABuilder::rawByte(const ParseNode& n) {
  const NFA::VertexDescriptor v = Fsm->addVertex();
  (*Fsm)[v].Trans = Fsm->TransFac->getByte(n.Val);
  TempFrag.initFull(v, n);
  Stack.push(TempFrag);
}

void NFABuilder::dot(const ParseNode&) {
  charClass(ParseNode(ParseNode::CHAR_CLASS, 0, 0x10FFFF));
}

void NFABuilder::charClass(const ParseNode& n) {
  const UnicodeSet uset(n.Set.CodePoints & Enc->validCodePoints());

  if (uset.none()) {
    if (!n.Set.Breakout.Additive || n.Set.Breakout.Bytes.none()) {
      THROW_RUNTIME_ERROR_WITH_CLEAN_OUTPUT(
        "intersection of character class with " << Enc->name() << " is empty"
      );
    }

    // the breakout bytes are the entire character class
    NFA::VertexDescriptor v = Fsm->addVertex();
    (*Fsm)[v].Trans = Fsm->TransFac->getSmallest(n.Set.Breakout.Bytes);
    TempFrag.initFull(v, n);
  }
  else {
    // convert the code point set into collapsed encoding ranges
    TempEncRanges.clear();
    Enc->write(uset, TempEncRanges);

    // handle the breakout bytes
    if (n.Set.Breakout.Bytes.any()) {
      if (n.Set.Breakout.Additive) {
        // add breakout bytes to encodings
        if (TempEncRanges[0].size() == 1) {
          TempEncRanges[0][0] |= n.Set.Breakout.Bytes;
        }
        else {
          TempEncRanges.emplace_back(1);
          TempEncRanges.back()[0] = n.Set.Breakout.Bytes;
        }
      }
      else {
        // subtract breakout bytes from encodings
        if (TempEncRanges[0].size() == 1) {
          TempEncRanges[0][0] &= ~n.Set.Breakout.Bytes;
          if (TempEncRanges[0][0].none()) {
            TempEncRanges.erase(TempEncRanges.begin());

            // ensure that at least one initial byte remains
            if (TempEncRanges.empty()) {
              THROW_RUNTIME_ERROR_WITH_CLEAN_OUTPUT(
                "intersection of character class with " << Enc->name()
                                                        << " is empty"
              );
            }
          }
        }
      }
    }

    ByteSet bs;
    TempFrag.reset(n);

    // create a graph from the collapsed ranges
    for (const std::vector<ByteSet>& enc : TempEncRanges) {
      NFA::VertexDescriptor head, tail;

      //
      // find a suffix of enc in this fragment
      //

      int32_t b = enc.size()-1;

      // find a match for the last transition
      const auto oi = std::find_if(
        TempFrag.OutList.begin(), TempFrag.OutList.end(),
        [&](const std::pair<NFA::VertexDescriptor,uint32_t>& p) {
          return (*Fsm)[p.first].Trans->getBytes(bs) == enc[b];
        }
      );

      if (oi != TempFrag.OutList.end()) {
        // match, use this tail
        tail = oi->first;

        // walk backwards until a transition mismatch
        for (--b; b >= 0; --b) {
          head = 0;
          for (const NFA::VertexDescriptor h : Fsm->inVertices(tail)) {
            (*Fsm)[h].Trans->getBytes(bs);
            if (bs == enc[b]) {
              tail = head = h;
              break;
            }
            head = 0;
          }

          if (!head) {
            // tail is as far back as we can go
            break;
          }
        }
      }
      else {
        // no match, build a new tail
        tail = Fsm->addVertex();
        (*Fsm)[tail].Trans = Fsm->TransFac->getSmallest(enc[b--]);
        TempFrag.OutList.emplace_back(tail, 0);
      }

      //
      // build from the start of enc to meet the existing suffix
      //

      for ( ; b >= 0; --b) {
        head = Fsm->addVertex();
        (*Fsm)[head].Trans = Fsm->TransFac->getSmallest(enc[b]);
        Fsm->addEdge(head, tail);
        tail = head;
      }

      TempFrag.InList.push_back(tail);
    }
  }

  Fsm->Deterministic = false;
  Stack.push(TempFrag);
}

void NFABuilder::question(const ParseNode&) {
  Fragment& optional = Stack.top();
  if (optional.Skippable > optional.InList.size()) {
    optional.Skippable = optional.InList.size();
  }
}

void NFABuilder::question_ng(const ParseNode&) {
  Fragment& optional = Stack.top();
  optional.Skippable = 0;
}

void NFABuilder::plus(const ParseNode& n) {
  Fragment& repeat = Stack.top();
  repeat.N = n;

  // make back edges
  patch_pre(repeat.OutList, repeat.InList);
}

void NFABuilder::plus_ng(const ParseNode& n) {
  Fragment& repeat = Stack.top();
  repeat.N = n;

  // make back edges
  patch_post(repeat.OutList, repeat.InList);
}

void NFABuilder::star(const ParseNode& n) {
  plus(n);
  question(n);
}

void NFABuilder::star_ng(const ParseNode& n) {
  plus_ng(n);
  question_ng(n);
}

void NFABuilder::repetition(const ParseNode& n) {
  if (n.Child.Rep.Min == 0) {
    if (n.Child.Rep.Max == 1) {
      question(n);
      return;
    }
    else if (n.Child.Rep.Max == UNBOUNDED) {
      star(n);
      return;
    }
  }
  else if (n.Child.Rep.Min == 1 && n.Child.Rep.Max == UNBOUNDED) {
    plus(n);
    return;
  }

  // all other cases are already reduced by traverse
}

void NFABuilder::repetition_ng(const ParseNode& n) {
  if (n.Child.Rep.Min == 0) {
    if (n.Child.Rep.Max == 1) {
      question_ng(n);
      return;
    }
    else if (n.Child.Rep.Max == UNBOUNDED) {
      star_ng(n);
      return;
    }
  }
  else if (n.Child.Rep.Min == 1 && n.Child.Rep.Max == UNBOUNDED) {
    plus_ng(n);
    return;
  }

  // all other cases are already reduced by traverse
}

void NFABuilder::alternate(const ParseNode& n) {
  Fragment second;
  second.assign(Stack.top());
  Stack.pop();

  Fragment& first(Stack.top());

  if (first.Skippable != NOSKIP) {
    // leave first.Skippable unchanged
  }
  else if (second.Skippable != NOSKIP) {
    first.Skippable = first.InList.size() + second.Skippable;
  }
  else {
    first.Skippable = NOSKIP;
  }

  first.InList.insert(first.InList.end(),
                      second.InList.begin(), second.InList.end());
  first.OutList.insert(first.OutList.end(),
                       second.OutList.begin(), second.OutList.end());

  first.N = n;
}

void NFABuilder::concatenate(const ParseNode& n) {
  TempFrag.assign(Stack.top());
  Stack.pop();
  Fragment& first(Stack.top());

  if (first.N.Type == ParseNode::LOOKBEHIND_NEG) {
    // this is not a real concatenation
    // mark the right as anchored at start
    for (NFA::VertexDescriptor v: TempFrag.InList) {
      (*Fsm)[v].AtStart = true;
    }

    // put the right back onto the stack
    first.assign(TempFrag);
  }
  else if (TempFrag.N.Type == ParseNode::LOOKAHEAD_NEG) {
    // this is not a real concatenation
    // mark the left as anchored at end
    for (NFA::VertexDescriptor v: first.InList) {
      (*Fsm)[v].AtEnd = true;
    }
  }
  else {
    // patch left out to right in
    patch_mid(first.OutList, TempFrag.InList, TempFrag.Skippable);

    // build new in list
    if (first.Skippable != NOSKIP) {
      first.InList.insert(first.InList.begin() + first.Skippable,
                          TempFrag.InList.begin(), TempFrag.InList.end());
    }

    // build new out list
    if (TempFrag.Skippable != NOSKIP) {
      first.OutList.insert(first.OutList.end(),
                           TempFrag.OutList.begin(), TempFrag.OutList.end());
    }
    else {
      first.OutList.swap(TempFrag.OutList);
    }

    // set new skippable
    first.Skippable = first.Skippable == NOSKIP || TempFrag.Skippable == NOSKIP
      ? NOSKIP : first.Skippable + TempFrag.Skippable;
  }

  first.N = n;
}

void NFABuilder::start_anchor(const ParseNode& n) {
  TempFrag.initFull(0, n);
  Stack.push(TempFrag);
}

void NFABuilder::end_anchor(const ParseNode& n) {
  TempFrag.initFull(0, n);
  Stack.push(TempFrag);
}

void NFABuilder::lookbehind_pos(const ParseNode& n) {
}

void NFABuilder::lookahead_pos(const ParseNode& n) {
}

void NFABuilder::finish(const ParseNode& n) {
  // Stack size MUST be 2 when we arrive here: The bottom of the stack is
  // the start fragment, the top is the fragment for the rest of the NFA.
  if (Stack.size() == 2) {
    concatenate(n);
    const Fragment& start(Stack.top());

    for (const auto& i: start.OutList) {
      // std::cout << "marking " << *it << " as a match" << std::endl;
      const NFA::VertexDescriptor v = i.first;
      if (v == 0) {
        // State 0 is not allowed to be a match state;
        // i.e. 0-length REs are not allowed
        reset();
        return;
      }
      else {
        NFA::Vertex& final((*Fsm)[v]);
        final.Label = CurLabel;
        final.IsMatch = true;
      }
    }
    // std::cout << "final is " << final << std::endl;
    IsGood = true;
    Stack.pop();
  }
  else {
    reset();
    // THROW_RUNTIME_ERROR_WITH_OUTPUT("Final parse stack size should be 2, was " << Stack.size());
  }
}

void NFABuilder::traverse(const ParseNode* root) {
  // holder for synthesized nodes
  std::vector<std::unique_ptr<ParseNode>> synth;

  // wind for preorder traversal, unwind for postorder
  std::stack<const ParseNode*> wind, unwind;
  wind.push(root);

  while (!wind.empty()) {
    const ParseNode* n = wind.top();
    wind.pop();
    unwind.push(n);

    if ((n->Type == ParseNode::REGEXP ||
         n->Type == ParseNode::ALTERNATION ||
         n->Type == ParseNode::CONCATENATION ||
         n->Type == ParseNode::REPETITION ||
         n->Type == ParseNode::REPETITION_NG) && n->Child.Left) {
      // This node has a left child
      if ((n->Type == ParseNode::REPETITION ||
           n->Type == ParseNode::REPETITION_NG) &&
         !((n->Child.Rep.Min == 0 &&
           (n->Child.Rep.Max == 1 || n->Child.Rep.Max == UNBOUNDED)) ||
           (n->Child.Rep.Min == 1 && n->Child.Rep.Max == UNBOUNDED)))
      {
        // This is a repetition, but not one of the special named ones.
        // We synthesize nodes here to eliminate counted repetitions.

        // NB: We expect that all empty ({0,0} and {0,0}?) and single
        // ({1,1}, {1,1}?) have been excised from the parse tree by now.

        //
        // T{n} = T...T
        //          ^
        //        n times
        //
        // T{n,} = T...TT*
        //           ^
        //         n times
        //
        // T{n,m} = T...TT?...T? = T...T(T(T...)?)?
        //            ^     ^
        //       n times   m-n times
        //
        // Note that the latter equivalence for T{n,m} produces
        // a graph with outdegree 2, while the former produces
        // one with outdegree m-n.
        //

        ParseNode root;
        ParseNode* none = 0;
        ParseNode* parent = &root;

        if (n->Child.Rep.Min > 0) {
          // build the mandatory part
          for (uint32_t i = 1; i < n->Child.Rep.Min; ++i) {
            synth.push_back(std::unique_ptr<ParseNode>(
              new ParseNode(ParseNode::CONCATENATION, n->Child.Left, none))
            );
            ParseNode* con = synth.back().get();

            parent->Child.Right = con;
            parent = con;
          }
        }

        if (n->Child.Rep.Min == n->Child.Rep.Max) {
          // finish the mandatory part
          parent->Child.Right = n->Child.Left;
        }
        else if (n->Child.Rep.Max == UNBOUNDED) {
          // build the unbounded optional part
          if (n->Child.Rep.Min == 0) {
            synth.push_back(std::unique_ptr<ParseNode>(
              new ParseNode(n->Type, n->Child.Left, 0, UNBOUNDED))
            );
            ParseNode* star = synth.back().get();
            parent->Child.Right = star;
          }
          else {
            synth.push_back(std::unique_ptr<ParseNode>(
              new ParseNode(n->Type, n->Child.Left, 1, UNBOUNDED))
            );
            ParseNode* plus = synth.back().get();
            parent->Child.Right = plus;
          }
        }
        else {
          if (n->Child.Rep.Min > 0) {
            // finish the mandatory part
            synth.push_back(std::unique_ptr<ParseNode>(
              new ParseNode(ParseNode::CONCATENATION, n->Child.Left, none))
            );
            ParseNode* con = synth.back().get();
            parent->Child.Right = con;
            parent = con;
          }

          // build the bounded optional part
          for (uint32_t i = 1; i < n->Child.Rep.Max - n->Child.Rep.Min; ++i) {
            synth.push_back(std::unique_ptr<ParseNode>(
              new ParseNode(ParseNode::CONCATENATION, n->Child.Left, none))
            );
            ParseNode* con = synth.back().get();

            synth.push_back(std::unique_ptr<ParseNode>(
              new ParseNode(n->Type, con, 0, 1))
            );
            ParseNode* question = synth.back().get();

            parent->Child.Right = question;
            parent = con;
          }

          synth.push_back(std::unique_ptr<ParseNode>(
            new ParseNode(n->Type, n->Child.Left, 0, 1))
          );
          ParseNode* question = synth.back().get();
          parent->Child.Right = question;
        }

        wind.push(root.Child.Right);
      }
      else {
        // this is not a repetition, or is one of ? * + ?? *? +?
        wind.push(n->Child.Left);
      }
    }

    if ((n->Type == ParseNode::ALTERNATION ||
         n->Type == ParseNode::CONCATENATION) && n->Child.Right) {
      wind.push(n->Child.Right);
    }
  }

  while (!unwind.empty()) {
    const ParseNode* n = unwind.top();
    unwind.pop();

    callback(*n);
  }
}

bool NFABuilder::build(const ParseTree& tree) {
  traverse(tree.Root);
  return IsGood;
}

void NFABuilder::callback(const ParseNode& n) {
  switch (n.Type) {
  case ParseNode::REGEXP:
    finish(n);
    break;
  case ParseNode::LOOKBEHIND_POS:
    lookbehind_pos(n);
    break;
  case ParseNode::LOOKBEHIND_NEG:
    start_anchor(n);
    break;
  case ParseNode::LOOKAHEAD_POS:
    lookahead_pos(n);
    break;
  case ParseNode::LOOKAHEAD_NEG:
    end_anchor(n);
    break;
  case ParseNode::ALTERNATION:
    alternate(n);
    break;
  case ParseNode::CONCATENATION:
    concatenate(n);
    break;
  case ParseNode::REPETITION:
    repetition(n);
    break;
  case ParseNode::REPETITION_NG:
    repetition_ng(n);
    break;
  case ParseNode::DOT:
    dot(n);
    break;
  case ParseNode::CHAR_CLASS:
    charClass(n);
    break;
  case ParseNode::LITERAL:
    literal(n);
    break;
  case ParseNode::BYTE:
    rawByte(n);
    break;
  default:
    break;
  }

/*
  std::cerr << "Stack size is " << Stack.size() << std::endl;
  if (Stack.size() > 0) {
     std::cerr << "top is " << Stack.top() << std::endl;
  }
*/
}
