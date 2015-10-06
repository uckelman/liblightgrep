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

#include <deque>
#include <vector>

#include "codegen.h"

void CodeGenVisitor::discover_vertex(NFA::VertexDescriptor v, const NFA& graph) {
  Helper.discover(v, graph);
}

uint32_t CodeGenVisitor::calcJumpTableSize(NFA::VertexDescriptor v, const NFA& graph, uint32_t outDegree) {
  if (outDegree > 3) {
    const TransitionTbl tbl(pivotStates(v, graph));
    if (maxOutbound(tbl) < outDegree) {
      uint32_t sizeIndirectTables = 0,
               num,
               first = 256,
               last  = 0;

      for (uint32_t i = 0; i < 256; ++i) {
        num = tbl[i].size();
        if (num > 1) {
          sizeIndirectTables += num;
        }
        if (num) {
          first = std::min(first, i);
          last  = i;
        }
      }

      Helper.Snippets[v].Op = JUMP_TABLE_RANGE_OP;
      // JumpTableRange instr + inclusive number
      return 2 + (last - first) + 2*sizeIndirectTables;
    }
  }
  return 0;
}

void CodeGenVisitor::finish_vertex(NFA::VertexDescriptor v, const NFA& graph) {
  // std::cerr << "on state " << v << " with discover rank " << Helper.DiscoverRanks[v] << std::endl;

  uint32_t size = 0;
  const uint32_t outDegree = graph.outDegree(v);

  // 1 for a label
  size += (graph[v].Label != NONE);

  if (graph[v].IsMatch) {
    // 1 for match, 1 for finish; or 1 for match, 2 for jump if
    // match is nonterminal
    size += 2 + (outDegree > 0);
  }

  // 1 for a start anchor, 1 for an end anchor
  size += graph[v].AtStart + graph[v].AtEnd;

  if (outDegree) {
    uint32_t outOps = calcJumpTableSize(v, graph, outDegree);

    if (outDegree < 4 || outOps == 0) {
      // count each of the non-initial children
      outOps += 2*(outDegree-1);

      // count the first child only if it needs a jump
      if (Helper.DiscoverRanks[v] + 1 !=
          Helper.DiscoverRanks[graph.outVertex(v, 0)]) {
        outOps += 2;
      }
    }

    size += outOps;
  }

  size += Helper.Snippets[v].CheckIndex != NONE;

  const uint32_t eval = graph[v].Trans ? graph[v].Trans->numInstructions() : 0;
  Helper.addSnippet(v, eval, size);
}

void specialVisit(const NFA& graph, NFA::VertexDescriptor startVertex, CodeGenVisitor& vis) {
  std::deque<NFA::VertexDescriptor> statesToVisit;
  std::vector<NFA::VertexDescriptor> inOrder;
  std::vector<bool> discovered(graph.verticesSize(), false);

  inOrder.reserve(graph.verticesSize());

  discovered[startVertex].flip();
  statesToVisit.push_back(startVertex);

  while (!statesToVisit.empty()) {
    const NFA::VertexDescriptor v = statesToVisit.front();
    statesToVisit.pop_front();

    vis.discover_vertex(v, graph);
    inOrder.push_back(v);

    const bool nobranch = graph.outDegree(v) < 2;

    for (const NFA::VertexDescriptor t : graph.outVertices(v)) {
      if (!discovered[t]) {
        discovered[t].flip();

        if (nobranch) {
          statesToVisit.push_front(t);
        }
        else {
          statesToVisit.push_back(t);
        }
      }
    }
  }

  for (const NFA::VertexDescriptor v : inOrder) {
    vis.finish_vertex(v, graph);
  }
}
