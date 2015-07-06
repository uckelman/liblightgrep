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

#include "parser.h"
#include "parsetree.h"
#include "rewriter.h"
#include "unparser.h"

SCOPE_TEST(hasZeroLengthMatch_a_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aQ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a?", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aS_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a*", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a+", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aQQ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a??", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aSQ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a*?", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aPQ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a+?", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_a0_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{0}", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_a1_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1}", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_a0_1_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{0,1}", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_a0__Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{0,}", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_a1__Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1,}", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_ab_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"ab", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aSb_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a*b", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_abS_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"ab*", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aSbS_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a*b*", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aOrb_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a|b", false, false}, tree));
  SCOPE_ASSERT(!hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aSOrb_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a*|b", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aOrbS_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a|b*", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(hasZeroLengthMatch_aSOrbS_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a*|b*", false, false}, tree));
  SCOPE_ASSERT(hasZeroLengthMatch(tree.Root));
}

SCOPE_TEST(spliceOutParentLeftTest) {
  ParseTree tree;
  tree.init(4);

  ParseNode *l = tree.add(ParseNode(ParseNode::LITERAL, 'l'));
  ParseNode *r = tree.add(ParseNode(ParseNode::LITERAL, 'r'));
  ParseNode *con = tree.add(ParseNode(ParseNode::CONCATENATION, l, r));
  tree.Root = tree.add(ParseNode(ParseNode::REGEXP, con));

  spliceOutParent(tree.Root, con, l);

  SCOPE_ASSERT_EQUAL(l, tree.Root->Child.Left);
  SCOPE_ASSERT_EQUAL((ParseNode*) nullptr, tree.Root->Child.Right);
}

SCOPE_TEST(spliceOutParentRightTest) {
  ParseTree tree;
  tree.init(4);

  ParseNode *l = tree.add(ParseNode(ParseNode::LITERAL, 'l'));
  ParseNode *r = tree.add(ParseNode(ParseNode::LITERAL, 'r'));
  ParseNode *con = tree.add(ParseNode(ParseNode::CONCATENATION, l, r));
  tree.Root = tree.add(ParseNode(ParseNode::REGEXP, con));

  spliceOutParent(tree.Root, con, r);

  SCOPE_ASSERT_EQUAL(r, tree.Root->Child.Left);
  SCOPE_ASSERT_EQUAL((ParseNode*) nullptr, tree.Root->Child.Right);
}

// FIXME: Split this into multiple tests.
SCOPE_TEST(reduceTrailingNongreedyThenEmptyTest) {
  ParseTree tree;

  SCOPE_ASSERT(parse({"a", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));

  SCOPE_ASSERT(parse({"a?", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a?", unparse(tree));

  SCOPE_ASSERT(parse({"a*", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a*", unparse(tree));

  SCOPE_ASSERT(parse({"a+", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a+", unparse(tree));

  SCOPE_ASSERT(parse({"a{0}", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));

  SCOPE_ASSERT(parse({"a{1}", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}", unparse(tree));

  SCOPE_ASSERT(parse({"a{0,1}", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a?", unparse(tree));

  SCOPE_ASSERT(parse({"a{0,}", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a*", unparse(tree));

  SCOPE_ASSERT(parse({"a{1,}", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a+", unparse(tree));

  SCOPE_ASSERT(parse({"a{1,1}", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}", unparse(tree));

  SCOPE_ASSERT(parse({"a{1,2}", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1,2}", unparse(tree));

  SCOPE_ASSERT(parse({"a??", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));

  SCOPE_ASSERT(parse({"a*?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));

  SCOPE_ASSERT(parse({"a+?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}", unparse(tree));

  SCOPE_ASSERT(parse({"a{0}?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));

  SCOPE_ASSERT(parse({"a{1}?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}", unparse(tree));

  SCOPE_ASSERT(parse({"a{0,1}?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));

  SCOPE_ASSERT(parse({"a{0,}?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));

  SCOPE_ASSERT(parse({"a{1,}?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}", unparse(tree));

  SCOPE_ASSERT(parse({"a{1,1}?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}", unparse(tree));

  SCOPE_ASSERT(parse({"a{1,2}?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}", unparse(tree));

  SCOPE_ASSERT(parse({"ab", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("ab", unparse(tree));

  SCOPE_ASSERT(parse({"a*b", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a*b", unparse(tree));

  SCOPE_ASSERT(parse({"ab*", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("ab*", unparse(tree));

  SCOPE_ASSERT(parse({"a*b*", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a*b*", unparse(tree));

  SCOPE_ASSERT(parse({"a|b", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a|b", unparse(tree));

  SCOPE_ASSERT(parse({"a*|b", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a*|b", unparse(tree));

  SCOPE_ASSERT(parse({"a|b*", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a|b*", unparse(tree));

  SCOPE_ASSERT(parse({"a*|b*", false, false}, tree));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a*|b*", unparse(tree));

  SCOPE_ASSERT(parse({"a+?b?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}b?", unparse(tree));

  SCOPE_ASSERT(parse({"a+?b*", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}b*", unparse(tree));

  SCOPE_ASSERT(parse({"(a|b)+?b*", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("(a|b){1}b*", unparse(tree));

  SCOPE_ASSERT(parse({"a+?(b|c*)", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}(b|c*)", unparse(tree));

  SCOPE_ASSERT(parse({"a+?b{0,5}", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}b{0,5}", unparse(tree));

  SCOPE_ASSERT(parse({"a{1,5}?b{0,1}", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}b?", unparse(tree));

  SCOPE_ASSERT(parse({"aa.+?a*", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("aa.{1}a*", unparse(tree));

  SCOPE_ASSERT(parse({"a.*?a*", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a.{0}a*", unparse(tree));
}

SCOPE_TEST(reduceTrailingNongreedyThenEmpty_aPQdotQQaSQ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a+?.??a*?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}.{0}a{0}", unparse(tree));
}

SCOPE_TEST(reduceTrailingNongreedyThenEmpty_PLaPQRPPQ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(a+?)+?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("(a{1}){1}", unparse(tree));
}

SCOPE_TEST(reduceTrailingNongreedyThenEmpty_aaaSQOraOraaSQ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"aaa*?|a|aa*?", false, false}, tree));
  SCOPE_ASSERT(reduceTrailingNongreedyThenEmpty(tree.Root));
  SCOPE_ASSERT_EQUAL("aaa{0}|a|aa{0}", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a", false, false}, tree));
  SCOPE_ASSERT(!reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a1_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1}", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a1_1_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1,1}", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a1_2_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1,2}", false, false}, tree));
  SCOPE_ASSERT(!reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1,2}", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a1Q_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1}?", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a2Q_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{2}?", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a{2}", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a1_2Q_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1,2}?", false, false}, tree));
  SCOPE_ASSERT(!reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1,2}?", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a1Orb1_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{1}|b{1}", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a|b", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a11_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(a{1}){1}", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a111_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"((a{1}){1}){1}", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a12_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(a{1}){2}", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a{2}", unparse(tree));
}

SCOPE_TEST(reduceUselessRepetitions_a121_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"((a{1}){2}){1}", false, false}, tree));
  SCOPE_ASSERT(reduceUselessRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("a{2}", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_a_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a", false, false}, tree));
  SCOPE_ASSERT(!reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_a0_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{0}", false, false}, tree));
  SCOPE_ASSERT(reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_a0b0_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{0}b{0}", false, false}, tree));
  SCOPE_ASSERT(reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_a0b_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a{0}b", false, false}, tree));
  SCOPE_ASSERT(reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("b", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_ba0_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"ba{0}", false, false}, tree));
  SCOPE_ASSERT(reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("b", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_aLPa0OraRP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a(a{0}|a)", false, false}, tree));
  SCOPE_ASSERT(reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("a", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_aLPaOra0RP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a(a|a{0})", false, false}, tree));
  SCOPE_ASSERT(reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("aa?", unparse(tree));
}

SCOPE_TEST(reduceEmptySubtrees_aaa0OraOraa0_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"aaa{0}|a|aa{0}", false, false}, tree));
  SCOPE_ASSERT(reduceEmptySubtrees(tree.Root));
  SCOPE_ASSERT_EQUAL("aa|a|a", unparse(tree));
}

SCOPE_TEST(combineConsecutiveRepetitions_LPabRPPLPabRPP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(ab)+(ab)+", false, false}, tree));
  SCOPE_ASSERT(combineConsecutiveRepetitions(tree.Root));
  SCOPE_ASSERT_EQUAL("(ab){2,}", unparse(tree));
}

SCOPE_TEST(reduceTrailingNongreedyThenGreedy_LPabRPQQab_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(ab)??ab", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT(reduceTrailingNongreedyThenGreedy(tree.Root));
  SCOPE_ASSERT_EQUAL("(ab){1}", unparse(tree));
}

SCOPE_TEST(reduceTrailingNongreedyThenGreedy_aDotaaaQQaDot_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a.aaa??a.", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT(!reduceTrailingNongreedyThenGreedy(tree.Root));
  SCOPE_ASSERT_EQUAL("a.aaa??a.", unparse(tree));
}

SCOPE_TEST(reduceTrailingNongreedyThenGreedy_aSQaOraSQa_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a*?a|a*?a", false, false}, tree));
  SCOPE_ASSERT(!makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT(reduceTrailingNongreedyThenGreedy(tree.Root));
  SCOPE_ASSERT_EQUAL("a{1}|a{1}", unparse(tree));
}

SCOPE_TEST(makeBinopsRightAssociative_LPabRPc_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(ab)c", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT_EQUAL("abc", unparse(tree));
}

SCOPE_TEST(makeBinopsRightAssociative_aLPbcRP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a(bc)", false, false}, tree));
  SCOPE_ASSERT(!makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT_EQUAL("abc", unparse(tree));
}

SCOPE_TEST(makeBinopsRightAssociative_LPaOrbRPOrc_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(a|b)|c", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT_EQUAL("a|b|c", unparse(tree));
}

SCOPE_TEST(makeBinopsRightAssociative_aOrLPbOrcRP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a|(b|c)", false, false}, tree));
  SCOPE_ASSERT(!makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT_EQUAL("a|b|c", unparse(tree));
}

SCOPE_TEST(makeBinopsRightAssociative_LPLPLPabRPcRPdRPe_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(((ab)c)d)e", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));

  /*
        r
        |
        &
       / \
      a   &
         / \
        b   &
           / \
          c   &
             / \
            d   e
  */

  ParseNode a(ParseNode::LITERAL, 'a'),
            b(ParseNode::LITERAL, 'b'),
            c(ParseNode::LITERAL, 'c'),
            d(ParseNode::LITERAL, 'd'),
            e(ParseNode::LITERAL, 'e'),
            de(ParseNode::CONCATENATION, &d, &e),
            cde(ParseNode::CONCATENATION, &c, &de),
            bcde(ParseNode::CONCATENATION, &b, &cde),
            abcde(ParseNode::CONCATENATION, &a, &bcde),
            root(ParseNode::REGEXP, &abcde);

  SCOPE_ASSERT_EQUAL(root, *tree.Root);
}

SCOPE_TEST(reduceNegativeLookarounds_LPLBaRP_Test) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a)", false, false}, tree));
  SCOPE_ASSERT(reduceNegativeLookarounds(tree.Root, tree));
  SCOPE_ASSERT_EQUAL("(?<!.)|(?<=[^a])", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookarounds_LPLBabRP_Test) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!ab)", false, false}, tree));
  SCOPE_ASSERT(reduceNegativeLookarounds(tree.Root, tree));
  SCOPE_ASSERT_EQUAL("(?<!.)|(?<=[^b])|(?<=((?<!.)|(?<=[^a]))b)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookarounds_LPLBaOrbRP_Test) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a|b)", false, false}, tree));
  SCOPE_ASSERT(reduceNegativeLookarounds(tree.Root, tree));
  SCOPE_ASSERT_EQUAL("((?<!.)|(?<=[^a]))((?<!.)|(?<=[^b]))", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookarounds_LPLAaRP_Test) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a)", false, false}, tree));
  SCOPE_ASSERT(reduceNegativeLookarounds(tree.Root, tree));
  SCOPE_ASSERT_EQUAL("(?=[^a])|(?!.)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookarounds_LPLAabRP_Test) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!ab)", false, false}, tree));
  SCOPE_ASSERT(reduceNegativeLookarounds(tree.Root, tree));
  SCOPE_ASSERT_EQUAL("(?=[^a])|(?!.)|(?=a((?=[^b])|(?!.)))", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookarounds_LPLAaOrbRP_Test) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a|b)", false, false}, tree));
  SCOPE_ASSERT(reduceNegativeLookarounds(tree.Root, tree));
  SCOPE_ASSERT_EQUAL("((?=[^a])|(?!.))((?=[^b])|(?!.))", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindLiteral_Dot) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!.)", false, false}, tree));
  reduceNegativeLookbehindLiteral(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!.)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindLiteral_a) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a)", false, false}, tree));
  reduceNegativeLookbehindLiteral(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!.)|(?<=[^a])", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindLiteral_byte_FF) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!\\zFF)", false, false}, tree));
  reduceNegativeLookbehindLiteral(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!.)|(?<=[^\\zFF])", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindAlternation_aOrb) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a|b)", false, false}, tree));
  reduceNegativeLookbehindAlternation(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!a)(?<!b)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindConcatenation_ab) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!ab)", false, false}, tree));
  reduceNegativeLookbehindConcatenation(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!b)|(?<=(?<!a)b)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindRepetition_aP) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a+)", false, false}, tree));
  reduceNegativeLookbehindRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindRepetition_aSP) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a*)", false, false}, tree));
  reduceNegativeLookbehindRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindRepetition_a2_17) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a{2,17})", false, false}, tree));
  reduceNegativeLookbehindRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!aa)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindRepetition_a3_17) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!a{3,17})", false, false}, tree));
  reduceNegativeLookbehindRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!aa{2})", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindLookaround_LBNa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!(?<!a))", false, false}, tree));
  reduceNegativeLookbehindLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<=a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindLookaround_LBPa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!(?<=a))", false, false}, tree));
  reduceNegativeLookbehindLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindLookaround_LANa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!(?!a))", false, false}, tree));
  reduceNegativeLookbehindLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?=a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookbehindLookaround_LAPa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?<!(?=a))", false, false}, tree));
  reduceNegativeLookbehindLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadLiteral_Dot) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!.)", false, false}, tree));
  reduceNegativeLookaheadLiteral(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!.)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadLiteral_a) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a)", false, false}, tree));
  reduceNegativeLookaheadLiteral(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?=[^a])|(?!.)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadLiteral_byte_FF) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!\\zFF)", false, false}, tree));
  reduceNegativeLookaheadLiteral(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?=[^\\zFF])|(?!.)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadAlternation_aOrb) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a|b)", false, false}, tree));
  reduceNegativeLookaheadAlternation(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!a)(?!b)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadConcatenation_ab) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!ab)", false, false}, tree));
  reduceNegativeLookaheadConcatenation(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!a)|(?=a(?!b))", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadRepetition_aP) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a+)", false, false}, tree));
  reduceNegativeLookaheadRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadRepetition_aSP) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a*)", false, false}, tree));
  reduceNegativeLookaheadRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("a{0}", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadRepetition_a2_17) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a{2,17})", false, false}, tree));
  reduceNegativeLookaheadRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!aa)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadRepetition_a3_17) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!a{3,17})", false, false}, tree));
  reduceNegativeLookaheadRepetition(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!aa{2})", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadLookaround_LBNa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!(?<!a))", false, false}, tree));
  reduceNegativeLookaheadLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<=a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadLookaround_LBPa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!(?<=a))", false, false}, tree));
  reduceNegativeLookaheadLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?<!a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadLookaround_LANa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!(?!a))", false, false}, tree));
  reduceNegativeLookaheadLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?=a)", unparse(tree));
}

SCOPE_TEST(reduceNegativeLookaheadLookaround_LAPa) {
  ParseTree tree;
  tree.init(1000);
  SCOPE_ASSERT(parse({"(?!(?=a))", false, false}, tree));
  reduceNegativeLookaheadLookaround(tree.Root->Child.Left, tree);
  SCOPE_ASSERT_EQUAL("(?!a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_aLPLBPaaRP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a(?<=aa)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=a)[a]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_aLPLBPaRP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a(?<=a)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[a]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPaRPa_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=a)a", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[a]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPaaRPa_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=aa)a", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[a](?=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPaaaaRPaaaa_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=aaaa)aaaa", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[a][a][a][a]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_aaaaLPLBPaaaaRP_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"aaaa(?<=aaaa)", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[a][a][a][a]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_WhackZa_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"\\Za", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[^\\z00-\\zFF]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_aWhackZ_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a\\Z", false, false}, tree));
  SCOPE_ASSERT(!shoveLookaroundsOutward(tree.Root));
}

SCOPE_TEST(shoveLookaroundsOutward_aWhackA_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"a\\A", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[^\\z00-\\zFF]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_WhackAa_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"\\Aa", false, false}, tree));
  SCOPE_ASSERT(!shoveLookaroundsOutward(tree.Root));
}

SCOPE_TEST(shoveLookaroundsOutward_WhackZaOrb_Test) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"\\Za|b", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("b", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLBPLPLBPaRPRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=(?<=a))", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLBPLPLAPaRPRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=(?=a))", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPLPLAPaRPRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=(?=a))", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPLPLBPaRPRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=(?<=a))", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPaRPP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=a)+", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPaRPPQ) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=a)+?", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPaRPS) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=a)*", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("x{0}", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPaRPSQ) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=a)*?", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("x{0}", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLBPLPLBPaRPbRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=(?<=a)b)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=ab)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLBPLPLBPaRPbbbbRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=(?<=a)bbbb)", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=abbbb)", unparse(tree));
}

SCOPE_TEST(shoeveLookaroundsOutward_LPLAPaLPLAPbRPRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=a(?=b))", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?=ab)", unparse(tree));
}

SCOPE_TEST(shoeveLookaroundsOutward_LPLAPaaaaLPLAPbRPRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=aaaa(?=b))", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?=aaaab)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLBPaLPLBPbRPRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=a(?<=b))", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[^\\z00-\\zFF]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAPLPLAPaRPbRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=(?=a)b)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("[^\\z00-\\zFF]", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAaRPOrLPLAPbRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=a)|(?=b)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?=a|b)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLBaRPOrLPLBPbRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?<=a)|(?<=b)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=a|b)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAaRPLPLBPbRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=a)(?<=b)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=b)(?=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAaRPLPLBPbRPLPLBcRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=a)(?<=b)(?<=c)", false, false}, tree));
  SCOPE_ASSERT(makeBinopsRightAssociative(tree.Root));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=b)(?<=c)(?=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_LPLAaRPWhackA) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"(?=a)\\A", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<!.)(?=a)", unparse(tree));
}

SCOPE_TEST(shoveLookaroundsOutward_WhackZLPLBaRP) {
  ParseTree tree;
  SCOPE_ASSERT(parse({"\\Z(?<=a)", false, false}, tree));
  SCOPE_ASSERT(shoveLookaroundsOutward(tree.Root));
  SCOPE_ASSERT_EQUAL("(?<=a)(?!.)", unparse(tree));
}

