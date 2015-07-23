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
#include <tuple>

bool hasZeroLengthMatch(const ParseNode* root);

bool reduceEmptySubtrees(ParseNode* root);
bool reduceUselessRepetitions(ParseNode* root);
bool reduceTrailingNongreedyThenEmpty(ParseNode* root);
bool reduceTrailingNongreedyThenGreedy(ParseNode* root);

bool combineConsecutiveRepetitions(ParseNode* root);
bool makeBinopsRightAssociative(ParseNode* root);

void spliceOutParent(ParseNode* gp, const ParseNode* p, ParseNode* c);

bool reduceNegativeLookarounds(ParseTree& tree);

void reduceNegativeLookbehindLiteral(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindConcatenation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindAlternation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindRepetition(ParseNode* n, ParseTree& tree);
void reduceNegativeLookbehindLookaround(ParseNode* n);

void reduceNegativeLookaheadLiteral(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadConcatenation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadAlternation(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadRepetition(ParseNode* n, ParseTree& tree);
void reduceNegativeLookaheadLookaround(ParseNode* n);

bool shoveLookaroundsOutward(ParseTree& tree);

size_t estimateNegativeLookaroundBlowup(const ParseNode* n);

std::tuple<ParseNode*,ParseNode*,ParseNode*> splitLookarounds(ParseNode* root);
