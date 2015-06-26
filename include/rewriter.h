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

#include "parsenode.h"
#include "parsetree.h"

#include <stack>

bool hasZeroLengthMatch(const ParseNode* root);

bool reduceEmptySubtrees(ParseNode* root);
bool reduceUselessRepetitions(ParseNode* root);
bool reduceTrailingNongreedyThenEmpty(ParseNode* root);
bool reduceTrailingNongreedyThenGreedy(ParseNode* root);

bool combineConsecutiveRepetitions(ParseNode* root);
bool makeBinopsRightAssociative(ParseNode* root);

void spliceOutParent(ParseNode* gp, const ParseNode* p, ParseNode* c);

ParseNode* previousAtom(std::stack<ParseNode*>& branch);

bool reduceNegativeLookarounds(ParseNode* root, ParseTree& tree);

void reduceNegativeLookbehindLiteral(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindConcatenation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindAlternation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindRepetition(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindLookaround(ParseNode* n, ParseTree& tree);

void reduceNegativeLookaheadLiteral(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadConcatenation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadAlternation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadRepetition(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadLookaround(ParseNode* n, ParseTree& tree);

bool flattenPositiveLookarounds(ParseNode* root);

bool shoveLookaroundsOutward(ParseNode* n, std::stack<ParseNode*>& branch);
bool shoveLookaroundsOutward(ParseNode* root);

