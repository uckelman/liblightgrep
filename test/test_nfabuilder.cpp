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

#include <scope/test.h>

#include "automata.h"
#include "instructions.h"
#include "nfabuilder.h"
#include "parser.h"
#include "parsetree.h"
#include "states.h"
#include "utility.h"
#include "encoders/concrete_encoders.h"

#include "test_helper.h"

SCOPE_TEST(parseAorB) {
  NFABuilder nfab;
  NFA& fsm(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"a|b", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
  SCOPE_ASSERT(fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);
}

SCOPE_TEST(parseAorBorC) {
  NFABuilder nfab;
  NFA& fsm(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"a|b|c", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(3u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(3));
  SCOPE_ASSERT(fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);
  SCOPE_ASSERT(fsm[3].IsMatch);
}

SCOPE_TEST(parseAB) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"ab", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);
}

SCOPE_TEST(parseAlternationAndConcatenation) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"a|bc", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(3));
  SCOPE_ASSERT(fsm[1].IsMatch);
  SCOPE_ASSERT(!fsm[2].IsMatch);
  SCOPE_ASSERT(fsm[3].IsMatch);
}

SCOPE_TEST(parseGroup) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"a(b|c)", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(3));
}

SCOPE_TEST(parseQuestionMark) {
  NFABuilder nfab;
  ParseTree tree;
  SCOPE_ASSERT(parse({"ab?", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  NFA& fsm(*nfab.getFsm());

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
}

SCOPE_TEST(parseQuestionMarkFirst) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"a?b", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
}

SCOPE_TEST(parseTwoQuestionMarks) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"ab?c?d", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(5u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.inDegree(0));
  // a
  SCOPE_ASSERT_EQUAL(3u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(1));
  // b?
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(2));
  // c?
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(3));
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(3));
  // d
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(4));
  SCOPE_ASSERT_EQUAL(3u, fsm.inDegree(4));
}

SCOPE_TEST(parseQuestionWithAlternation) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"(a|b?)c", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(3u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.inDegree(0));
  // a
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(1));
  // b?
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(2));
  // c
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(3));
  SCOPE_ASSERT_EQUAL(3u, fsm.inDegree(3));
}

SCOPE_TEST(parseQuestionWithGrouping) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"a(bc)?d", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(5u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  // a
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(1));
  // b
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  // c
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(3));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(3));
  // d
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(4));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(4));
}

SCOPE_TEST(parsePlus) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"a+", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.inDegree(0));
  // a+
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(1));
}

SCOPE_TEST(parseaPQb) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({"a+?b", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(1, 1));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(2));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(g[2].IsMatch);
}

SCOPE_TEST(parseStar) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"ab*c", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(2));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(3));
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(3));
}

SCOPE_TEST(parseStarWithGrouping) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"a(bc)*d", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(5u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  // a
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(1));
  // b
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  // c
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(3));
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(3));
  // d
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(4));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(4));
}

SCOPE_TEST(parseaQQb) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({"a??b", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 1));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(1));
  SCOPE_ASSERT(edgeExists(g, 1, 2));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(2));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(g[2].IsMatch);
}

SCOPE_TEST(parseaQQbQQc) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({"a??b??c", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(3u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 1));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 2));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 1));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT(edgeExists(g, 2, 3));

  SCOPE_ASSERT_EQUAL(3u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(g[3].IsMatch);
}

SCOPE_TEST(parseaQQbQc) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({"a??b?c", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(3u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(0, 1));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 2));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(1, 1));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT(edgeExists(g, 2, 3));

  SCOPE_ASSERT_EQUAL(3u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(g[3].IsMatch);
}

SCOPE_TEST(parseaQQOrbQQc) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({R"((a??|b??)c)", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(3u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 2));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(1));
  SCOPE_ASSERT(edgeExists(g, 1, 3));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT(edgeExists(g, 2, 3));

  SCOPE_ASSERT_EQUAL(3u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(g[3].IsMatch);
}

SCOPE_TEST(parseaOrbQa) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({"(a|b?)a", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(3u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 1));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(0, 2));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(1));
  SCOPE_ASSERT(edgeExists(g, 1, 3));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT(edgeExists(g, 2, 3));

  SCOPE_ASSERT_EQUAL(3u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(g[3].IsMatch);
}

SCOPE_TEST(parseaOrbQQa) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({R"((a|b??)a)", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(3u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(0, 1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 2));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(1));
  SCOPE_ASSERT(edgeExists(g, 1, 3));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT(edgeExists(g, 2, 3));

  SCOPE_ASSERT_EQUAL(3u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(g[3].IsMatch);
}

SCOPE_TEST(parseaSQb) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& g(*nfab.getFsm());

  SCOPE_ASSERT(parse({"a*?b", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 1));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(1, 1));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(2));

  SCOPE_ASSERT(!g[0].Trans);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(g[2].IsMatch);
}

SCOPE_TEST(parseDot) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({".+", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(1));

  ByteSet expected{{0x00,0x80}}, actual;
  fsm[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseHexCode) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\x20", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));

  ByteSet set;
  fsm[1].Trans->getBytes(set);
  SCOPE_ASSERT_EQUAL(1u, set.count());
  SCOPE_ASSERT(set[' ']);
}

SCOPE_TEST(parseHexDotPlus) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\x20\\zFF.+\\x20", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(5u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(3));
  SCOPE_ASSERT_EQUAL(2u, fsm.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(4));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(4));
}

SCOPE_TEST(parse2ByteUnicode) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  nfab.setEncoder(std::shared_ptr<Encoder>(new UTF16LE));
  SCOPE_ASSERT(parse({"ab", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(5u, fsm.verticesSize());
}

SCOPE_TEST(parseHighHex) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\zE5", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());

  ByteSet expected, actual;
  expected.set(0xE5);
  fsm[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseSimpleCharClass) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"[AaBb]", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));

  ByteSet expected, actual;
  expected.set('A');
  expected.set('a');
  expected.set('B');
  expected.set('b');
  fsm[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
  SCOPE_ASSERT_EQUAL("ABab/0", fsm[1].label());
}

SCOPE_TEST(parseUnprintableCharClass) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"[A\\zFF\\z00]", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));

  ByteSet expected, actual;
  expected.set('A');
  expected.set(0x00);
  expected.set(0xFF);
  fsm[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
  SCOPE_ASSERT_EQUAL("\\x00A\\xFF/0", fsm[1].label());
}

SCOPE_TEST(parseNegatedRanges) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"[^a-zA-Z0-9]", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));

  ByteSet expected, actual;

  expected.set();
  expected.set('0', '9' + 1, false);
  expected.set('A', 'Z' + 1, false);
  expected.set('a', 'z' + 1, false);
  expected.set(0x80, 0x100, false); // out-of-range

  fsm[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseCaseInsensitive) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"ab", false, true}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(0u, fsm.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(2));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
  Instruction i;
  SCOPE_ASSERT(fsm[1].Trans->toInstruction(&i));
  SCOPE_ASSERT_EQUAL(Instruction::makeEither('A', 'a'), i);
  SCOPE_ASSERT(fsm[2].Trans->toInstruction(&i));
  SCOPE_ASSERT_EQUAL(Instruction::makeEither('B', 'b'), i);
}

SCOPE_TEST(parseCaseInsensitiveCC) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"[a-z]", false, true}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(0u, fsm.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.inDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(1));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(fsm[1].IsMatch);

  SCOPE_ASSERT(!fsm[0].Trans);

  ByteSet ebs, abs;

  ebs.set('A', 'Z' + 1, true);
  ebs.set('a', 'z' + 1, true);

  fsm[1].Trans->getBytes(abs);
  SCOPE_ASSERT_EQUAL(ebs, abs);
}

SCOPE_TEST(parseSZeroMatchState) {
  NFABuilder nfab;
  ParseTree tree;
  SCOPE_ASSERT(parse({"a?", false, false}, tree));
  SCOPE_ASSERT(!nfab.build(tree));
}

SCOPE_TEST(parseRepeatedSkippables) {
  // we'll simulate a?b*
  NFABuilder nfab;
  SCOPE_ASSERT_EQUAL(1u, nfab.stack().size());
  nfab.callback(ParseNode(ParseNode::LITERAL, 'a'));
  SCOPE_ASSERT_EQUAL(2u, nfab.stack().size());
  SCOPE_ASSERT_EQUAL(NOSKIP, nfab.stack().top().Skippable);
  nfab.callback(ParseNode(ParseNode::REPETITION, nullptr, 0, 1));
  SCOPE_ASSERT_EQUAL(2u, nfab.stack().size());
  SCOPE_ASSERT_EQUAL(1u, nfab.stack().top().Skippable);
  nfab.callback(ParseNode(ParseNode::LITERAL, 'b'));
  SCOPE_ASSERT_EQUAL(3u, nfab.stack().size());
  SCOPE_ASSERT_EQUAL(NOSKIP, nfab.stack().top().Skippable);
  nfab.callback(ParseNode(ParseNode::REPETITION, nullptr, 0, UNBOUNDED));
  SCOPE_ASSERT_EQUAL(3u, nfab.stack().size());
  SCOPE_ASSERT_EQUAL(1u, nfab.stack().top().Skippable);
  nfab.callback(ParseNode(ParseNode::CONCATENATION, nullptr, nullptr));
  SCOPE_ASSERT_EQUAL(2u, nfab.stack().size());
  SCOPE_ASSERT_EQUAL(2u, nfab.stack().top().Skippable);
  nfab.callback(ParseNode(ParseNode::CONCATENATION, nullptr, nullptr));
  SCOPE_ASSERT_EQUAL(1u, nfab.stack().size());
  SCOPE_ASSERT_EQUAL(NOSKIP, nfab.stack().top().Skippable);
}

SCOPE_TEST(parseZeroDotStarZero) {
  NFABuilder nfab;
  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"0.*0", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(1, 0));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(1, 1));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(2u, g.inVertex(2, 0));
  SCOPE_ASSERT_EQUAL(1u, g.inVertex(2, 1));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(2));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(2, 0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(2, 1));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(1u, g.inVertex(3, 0));
  SCOPE_ASSERT_EQUAL(2u, g.inVertex(3, 1));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));
}

#define TEST_REPETITION_N(pattern, n) \
  std::ostringstream ss; \
  ss << pattern << '{' << n << '}'; \
\
  NFABuilder nfab; \
  NFA& g(*nfab.getFsm()); \
  ParseTree tree; \
  SCOPE_ASSERT(parse({ss.str(), false, false}, tree)); \
  SCOPE_ASSERT(nfab.build(tree)); \
\
  SCOPE_ASSERT_EQUAL(n + 1, g.verticesSize()); \
\
  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0)); \
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0)); \
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0)); \
\
  for (uint32_t i = 1; i < n; ++i) { \
    SCOPE_ASSERT_EQUAL(1u, g.inDegree(i)); \
    SCOPE_ASSERT_EQUAL(1u, g.outDegree(i)); \
    SCOPE_ASSERT_EQUAL(i+1, g.outVertex(i, 0)); \
    SCOPE_ASSERT(!g[i].IsMatch); \
  } \
\
  SCOPE_ASSERT_EQUAL(1u, g.inDegree(n)); \
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(n)); \
  SCOPE_ASSERT(g[n].IsMatch);

SCOPE_TEST(parse_aLCnRC) {
  for (uint32_t c = 1; c < 100; ++c) {
    TEST_REPETITION_N("a", c);
  }
}

#define TEST_REPETITION_N_U(pattern, n) \
  std::ostringstream ss; \
  ss << pattern << '{' << n << ",}"; \
\
  NFABuilder nfab; \
  NFA& g(*nfab.getFsm()); \
  ParseTree tree; \
  SCOPE_ASSERT(parse({ss.str(), false, false}, tree)); \
  SCOPE_ASSERT(nfab.build(tree)); \
\
  SCOPE_ASSERT_EQUAL(n + 1, g.verticesSize()); \
\
  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0)); \
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0)); \
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0)); \
\
  for (uint32_t i = 1; i < n; ++i) { \
    SCOPE_ASSERT_EQUAL(1u, g.inDegree(i)); \
    SCOPE_ASSERT_EQUAL(1u, g.outDegree(i)); \
    SCOPE_ASSERT_EQUAL(i+1, g.outVertex(i, 0)); \
    SCOPE_ASSERT(!g[i].IsMatch); \
  } \
\
  SCOPE_ASSERT_EQUAL(2u, g.inDegree(n)); \
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(n)); \
  SCOPE_ASSERT_EQUAL(n, g.outVertex(n, 0)); \
  SCOPE_ASSERT(g[n].IsMatch);

SCOPE_TEST(parse_aLCn_RC) {
  for (uint32_t n = 1; n < 100; ++n) {
    TEST_REPETITION_N_U("a", n);
  }
}

SCOPE_TEST(parse_aLC0_RCQb) {
  NFABuilder nfab;
  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{0,}?b", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 1));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(1, 1));
  SCOPE_ASSERT(!g[1].IsMatch);

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(2));
  SCOPE_ASSERT(g[2].IsMatch);
}

#define TEST_REPETITION_NG_N_U(pattern, n) \
  std::ostringstream ss; \
  ss << pattern << '{' << n << ",}?b"; \
\
  NFABuilder nfab; \
  NFA& g(*nfab.getFsm()); \
  ParseTree tree; \
  SCOPE_ASSERT(parse({ss.str(), false, false}, tree)); \
  SCOPE_ASSERT(nfab.build(tree)); \
\
  SCOPE_ASSERT_EQUAL(n + 2, g.verticesSize()); \
\
  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0)); \
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0)); \
  SCOPE_ASSERT_EQUAL(1, g.outVertex(0, 0)); \
\
  for (uint32_t i = 1; i < n-1; ++i) { \
    SCOPE_ASSERT_EQUAL(1u, g.inDegree(i)); \
    SCOPE_ASSERT_EQUAL(1u, g.outDegree(i)); \
    SCOPE_ASSERT_EQUAL(i+1, g.outVertex(i, 0)); \
    SCOPE_ASSERT(!g[i].IsMatch); \
  } \
\
  SCOPE_ASSERT_EQUAL(2u, g.inDegree(n)); \
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(n)); \
  SCOPE_ASSERT_EQUAL(n+1, g.outVertex(n, 0)); \
  SCOPE_ASSERT_EQUAL(n, g.outVertex(n, 1)); \
  SCOPE_ASSERT(!g[n].IsMatch); \
\
  SCOPE_ASSERT_EQUAL(1u, g.inDegree(n+1)); \
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(n+1)); \
  SCOPE_ASSERT(g[n].IsMatch);

SCOPE_TEST(parse_aLCn_RCQb) {
  for (uint32_t n = 1; n < 100; ++n) {
    TEST_REPETITION_N_U("a", n);
  }
}

SCOPE_TEST(parse_xa0_) {
  NFABuilder nfab;
  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"xa{0,}", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(2, 0));

  SCOPE_ASSERT(g[1].IsMatch);
  SCOPE_ASSERT(g[2].IsMatch);
}

#define TEST_REPETITION_N_M(pattern, n, m) \
  std::ostringstream ss; \
  ss << pattern << '{' << n << ',' << m << '}'; \
\
  NFABuilder nfab; \
  NFA& g(*nfab.getFsm()); \
  ParseTree tree; \
  SCOPE_ASSERT(parse({ss.str(), false, false}, tree)); \
  SCOPE_ASSERT(nfab.build(tree)); \
\
  SCOPE_ASSERT_EQUAL(m + 1, g.verticesSize()); \
\
  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0)); \
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0)); \
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0)); \
\
  for (uint32_t i = 1; i < n; ++i) { \
    SCOPE_ASSERT_EQUAL(1u, g.inDegree(i)); \
    SCOPE_ASSERT_EQUAL(1u, g.outDegree(i)); \
    SCOPE_ASSERT_EQUAL(i+1, g.outVertex(i, 0)); \
    SCOPE_ASSERT(!g[i].IsMatch); \
  } \
\
  for (uint32_t i = n; i < m; ++i) { \
    SCOPE_ASSERT_EQUAL(1u, g.inDegree(i)); \
    SCOPE_ASSERT_EQUAL(1u, g.outDegree(i)); \
    SCOPE_ASSERT_EQUAL(i+1, g.outVertex(i, 0)); \
    SCOPE_ASSERT(g[i].IsMatch); \
  } \
\
  SCOPE_ASSERT_EQUAL(1u, g.inDegree(m)); \
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(m)); \
  SCOPE_ASSERT(g[m].IsMatch);

SCOPE_TEST(parse_aLCn_mRC) {
  for (uint32_t n = 1; n < 5; ++n) {
    for (uint32_t m = n; m < 5; ++m) {
      TEST_REPETITION_N_M("a", n, m);
    }
  }
}

SCOPE_TEST(parse_aaQQb) {
  NFABuilder nfab;
  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"aa??b", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 1));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(2, 0));

  SCOPE_ASSERT_EQUAL(2u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));

  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(g[3].IsMatch);
}

SCOPE_TEST(parse_xLPaORaQQRPy) {
  NFABuilder nfab;
  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({R"(x(a|a??)y)", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(5u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(3u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));
  SCOPE_ASSERT_EQUAL(4u, g.outVertex(1, 1));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(1, 2));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT_EQUAL(4u, g.outVertex(2, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(3));
  SCOPE_ASSERT_EQUAL(4u, g.outVertex(3, 0));

  SCOPE_ASSERT_EQUAL(3u, g.inDegree(4));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(4));

  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(!g[3].IsMatch);
  SCOPE_ASSERT(g[4].IsMatch);
}

SCOPE_TEST(parseEncodingByteBreakout) {
  // 0x80 is not a valid UTF-8 byte by itself, we use \z to break it out

  NFABuilder nfab;
  nfab.setEncoder(std::shared_ptr<Encoder>(new UTF8));

  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"\\z80", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(1, 0));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(1));

  SCOPE_ASSERT(!g[0].IsMatch);
  SCOPE_ASSERT(g[1].IsMatch);

  SCOPE_ASSERT(!g[0].Trans);

  ByteSet expected, actual;
  expected.set(0x80);
  g[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseEncodingNotByteBreakout) {
  // 0x80 is a valid UTF-8 code point, referred to by \x{80}

  NFABuilder nfab;
  nfab.setEncoder(std::shared_ptr<Encoder>(new UTF8));

  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"\\x{80}", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(1, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(1, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.inVertex(2, 0));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(2));

  SCOPE_ASSERT(!g[0].IsMatch);
  SCOPE_ASSERT(!g[1].IsMatch);
  SCOPE_ASSERT(g[2].IsMatch);

  SCOPE_ASSERT(!g[0].Trans);

  ByteSet expected, actual;
  expected.set(0xC2);
  g[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);

  expected.reset();
  actual.reset();

  expected.set(0x80);
  g[2].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseEncodingCCCodePointWithBreakout) {
  NFABuilder nfab;
  nfab.setEncoder(std::shared_ptr<Encoder>(new UTF8));

  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"[A\\zFF]", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(1, 0));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(1));

  SCOPE_ASSERT(!g[0].IsMatch);
  SCOPE_ASSERT(g[1].IsMatch);

  SCOPE_ASSERT(!g[0].Trans);

  ByteSet expected{'A', 0xFF}, actual;
  g[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseEncodingCCCodePoint2ByteWithBreakout) {
  NFABuilder nfab;
  nfab.setEncoder(std::shared_ptr<Encoder>(new UTF8));

  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"[\\x{80}\\zFF]", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, g.outVertex(0, 0));
  SCOPE_ASSERT_EQUAL(3u, g.outVertex(0, 1));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(2u, g.inVertex(1, 0));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(1));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(2));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(2, 0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(2));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(2, 0));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(3));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(3, 0));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(3));

  SCOPE_ASSERT(!g[0].IsMatch);
  SCOPE_ASSERT(g[1].IsMatch);
  SCOPE_ASSERT(!g[2].IsMatch);
  SCOPE_ASSERT(g[3].IsMatch);

  SCOPE_ASSERT(!g[0].Trans);

  ByteSet expected{0x80}, actual;
  g[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);

  expected.reset();
  actual.reset();

  expected.set(0xC2);
  g[2].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);

  expected.reset();
  actual.reset();

  expected.set(0xFF);
  g[3].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseEncodingCCBreakoutOnly) {
  NFABuilder nfab;
  nfab.setEncoder(std::shared_ptr<Encoder>(new UTF8));

  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"[\\zFF]", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 1));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(1, 0));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(1));

  SCOPE_ASSERT(!g[0].IsMatch);
  SCOPE_ASSERT(g[1].IsMatch);

  SCOPE_ASSERT(!g[0].Trans);

  ByteSet expected{0xFF}, actual;
  g[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parseEncodingNegCCCodePointWithBreakout) {
  NFABuilder nfab;
  nfab.setEncoder(std::shared_ptr<Encoder>(new UTF8));

  NFA& g(*nfab.getFsm());
  ParseTree tree;
  SCOPE_ASSERT(parse({"[^\\x{02}-\\x{10FFFF}\\x01]", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(2u, g.verticesSize());

  SCOPE_ASSERT_EQUAL(0u, g.inDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, g.outVertex(0, 1));

  SCOPE_ASSERT_EQUAL(1u, g.inDegree(1));
  SCOPE_ASSERT_EQUAL(0u, g.inVertex(1, 0));
  SCOPE_ASSERT_EQUAL(0u, g.outDegree(1));

  SCOPE_ASSERT(!g[0].IsMatch);
  SCOPE_ASSERT(g[1].IsMatch);

  SCOPE_ASSERT(!g[0].Trans);

  ByteSet expected{0x00}, actual;
  g[1].Trans->getBytes(actual);
  SCOPE_ASSERT_EQUAL(expected, actual);
}

SCOPE_TEST(parse_WhackAa) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\Aa", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(!fsm[2].AtEnd);
}

SCOPE_TEST(parse_aWhackZ) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"a\\Z", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(!fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(fsm[2].AtEnd);
}

SCOPE_TEST(parse_WhackAaWhackZ) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\Aa\\Z", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(4u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(3));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(!fsm[2].IsMatch);
  SCOPE_ASSERT(fsm[3].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);
  SCOPE_ASSERT(!fsm[3].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(!fsm[2].AtEnd);
  SCOPE_ASSERT(fsm[3].AtEnd);
}

SCOPE_TEST(parse_WhackAaOrbWhackZ) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\Aa|b\\Z", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(5u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(3));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(4));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);
  SCOPE_ASSERT(!fsm[3].IsMatch);
  SCOPE_ASSERT(fsm[4].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);
  SCOPE_ASSERT(!fsm[3].AtStart);
  SCOPE_ASSERT(!fsm[4].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(!fsm[2].AtEnd);
  SCOPE_ASSERT(!fsm[3].AtEnd);
  SCOPE_ASSERT(fsm[4].AtEnd);
}

SCOPE_TEST(parse_WhackAWhackwP) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\A\\w+", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(!fsm[2].AtEnd);
}

SCOPE_TEST(parse_WhackwPWhackZ) {
  NFABuilder nfab;
  ParseTree tree;
  NFA& fsm(*nfab.getFsm());
  SCOPE_ASSERT(parse({"\\w+\\Z", false, false}, tree));
  SCOPE_ASSERT(nfab.build(tree));

  SCOPE_ASSERT_EQUAL(3u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(2));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(fsm[2].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(!fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(fsm[2].AtEnd);
}

SCOPE_TEST(parse_CaretWhackwP) {
  const ParseTree tree = std::get<1>(parseAndReduce({"^\\w+", false, false}));
  NFABuilder nfab;
  SCOPE_ASSERT(nfab.build(tree));
  NFA& fsm(*nfab.getFsm());

  SCOPE_ASSERT_EQUAL(4u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(2u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(3));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(!fsm[2].IsMatch);
  SCOPE_ASSERT(fsm[3].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);
  SCOPE_ASSERT(!fsm[3].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(!fsm[2].AtEnd);
  SCOPE_ASSERT(!fsm[3].AtEnd);
}

SCOPE_TEST(parse_LPLBNxyRPz) {
  const ParseTree tree = std::get<1>(parseAndReduce({"(?<!xy)z", false, false}));
  NFABuilder nfab;
  SCOPE_ASSERT(nfab.build(tree));
  NFA& fsm(*nfab.getFsm());

  SCOPE_ASSERT_EQUAL(7u, fsm.verticesSize());
  SCOPE_ASSERT_EQUAL(4u, fsm.outDegree(0));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(1));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(2));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(3));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(4));
  SCOPE_ASSERT_EQUAL(1u, fsm.outDegree(5));
  SCOPE_ASSERT_EQUAL(0u, fsm.outDegree(6));

  SCOPE_ASSERT(!fsm[0].IsMatch);
  SCOPE_ASSERT(!fsm[1].IsMatch);
  SCOPE_ASSERT(!fsm[2].IsMatch);
  SCOPE_ASSERT(!fsm[3].IsMatch);
  SCOPE_ASSERT(!fsm[4].IsMatch);
  SCOPE_ASSERT(!fsm[5].IsMatch);
  SCOPE_ASSERT(fsm[6].IsMatch);

  SCOPE_ASSERT(!fsm[0].AtStart);
  SCOPE_ASSERT(fsm[1].AtStart);
  SCOPE_ASSERT(!fsm[2].AtStart);
  SCOPE_ASSERT(fsm[3].AtStart);
  SCOPE_ASSERT(!fsm[4].AtStart);
  SCOPE_ASSERT(!fsm[5].AtStart);
  SCOPE_ASSERT(!fsm[6].AtStart);

  SCOPE_ASSERT(!fsm[0].AtEnd);
  SCOPE_ASSERT(!fsm[1].AtEnd);
  SCOPE_ASSERT(!fsm[2].AtEnd);
  SCOPE_ASSERT(!fsm[3].AtEnd);
  SCOPE_ASSERT(!fsm[4].AtEnd);
  SCOPE_ASSERT(!fsm[5].AtEnd);
  SCOPE_ASSERT(!fsm[6].AtEnd);

  SCOPE_ASSERT(!fsm[0].Assert);
  SCOPE_ASSERT(!fsm[1].Assert);
  SCOPE_ASSERT(fsm[2].Assert);
  SCOPE_ASSERT(!fsm[3].Assert);
  SCOPE_ASSERT(!fsm[4].Assert);
  SCOPE_ASSERT(fsm[5].Assert);
  SCOPE_ASSERT(!fsm[6].Assert);
}
