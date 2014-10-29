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

#include "codegen.h"
#include "utility.h"

#include <algorithm>
#include <iomanip>

uint32_t find_lmin(const NFA& graph) {
  uint32_t lmin = std::numeric_limits<uint32_t>::max();

  uint32_t depth;
  NFA::VertexDescriptor h;

  std::set<std::pair<uint32_t,NFA::VertexDescriptor>> next;
  next.emplace(0, 0);

  while (!next.empty()) {
    std::tie(depth, h) = *next.begin();
    next.erase(next.begin());

    if (depth > lmin) {
      break;
    }

    if (graph[h].IsMatch && depth < lmin) {
      lmin = depth;
    }

    // put successors in the heap if they're at lmin or less
    if (depth < lmin) {
      for (const NFA::VertexDescriptor t : graph.outVertices(h)) {
        next.emplace(depth+1, t);
      }
    }
  }

  return lmin;
}

std::pair<std::set<std::tuple<NFA::VertexDescriptor,uint32_t,std::vector<ByteSet>>>,size_t> prefixPrep(const NFA& graph) {
  const uint32_t lmin = find_lmin(graph);

  // do a depth-first traversal for rooted paths of length lmin
  std::set<std::tuple<NFA::VertexDescriptor,uint32_t,std::vector<ByteSet>>> paths;

  std::vector<std::tuple<NFA::VertexDescriptor,uint32_t,ByteSet>> br;
  br.emplace_back(0, NOLABEL, ByteSet());

  NFA::VertexDescriptor h;
  uint32_t label;

  do {
    std::tie(h, label, std::ignore) = br.back();

    if (br.size() == lmin + 1) {
      std::vector<ByteSet> pr;
      for (auto i = br.begin() + 1; i != br.end(); ++i) {
        pr.push_back(std::get<2>(*i));
      }
      paths.insert(std::make_tuple(h, label, pr));

      while (br.size() > 1) {
        br.pop_back();

        // find index of h in its parent p
        const NFA::VertexDescriptor p = std::get<0>(br.back());
        size_t hi = graph.outDegree(p) - 1;
        while (h != graph.outVertex(p, hi)) { --hi; }

        if (hi < graph.outDegree(p) - 1) {
          // h has a younger sibling
          const NFA::VertexDescriptor s = graph.outVertex(p, hi+1);
          label = graph[s].Label != NOLABEL ? graph[s].Label : label;
          ByteSet bs;
          graph[s].Trans->orBytes(bs);
          br.emplace_back(s, label, bs);
          break;
        }
        else {
          // h has no younger sibling; back up to parent p
          h = p;
        }
      }
    }
    else {
      // not at lmin, so there must be a child
      ByteSet bs;
      const NFA::VertexDescriptor t = graph.outVertex(h, 0);
      label = graph[t].Label != NOLABEL ? graph[t].Label : label;
      graph[t].Trans->orBytes(bs);
      br.emplace_back(t, label, bs);
    }
  } while (br.size() > 1);

  std::vector<std::tuple<NFA::VertexDescriptor,uint32_t,std::vector<ByteSet>>> prf, prfbefore(paths.begin(), paths.end());
  paths.clear();
  
  while (true) {
    for (auto p0 = prfbefore.begin(); p0 != prfbefore.end(); ++p0) {
      bool done = false;
      for (auto p1 = prf.begin(); p1 != prf.end(); ++p1) {
        if (std::get<0>(*p0) != std::get<0>(*p1)) {
          continue;
        }

        bool supset = true;
        for (size_t i = 0; i < std::get<2>(*p0).size(); ++i) {
          if (static_cast<std::bitset<256>>(std::get<2>(*p1)[i]) != (std::get<2>(*p0)[i] | std::get<2>(*p1)[i])) {
            // p1 not supset of p0
            supset = false;
            break;
          }
        }
      
        if (supset) {
          // discard p0
          done = true;
          break;
        }
        
        bool subset = true;
        for (size_t i = 0; i < std::get<2>(*p0).size(); ++i) {
          if (static_cast<std::bitset<256>>(std::get<2>(*p0)[i]) != (std::get<2>(*p0)[i] | std::get<2>(*p1)[i])) {
            // p1 not subset of p0
            subset = false;
            break;
          }
        }

        if (subset) {
          // replace p1
          std::get<2>(*p1) = std::get<2>(*p0);
          done = true;
          break;  
        }

        for (size_t i = 0; i < std::get<2>(*p0).size(); ++i) {
          if (std::equal(std::get<2>(*p0).begin(), std::get<2>(*p0).begin()+i, std::get<2>(*p1).begin()) && std::equal(std::get<2>(*p0).begin()+i+1, std::get<2>(*p0).end(), std::get<2>(*p1).begin()+i+1)) {
            std::get<2>(*p1)[i] |= std::get<2>(*p0)[i];
            done = true;
            break;
          }
        }

        if (done) {
          break;
        } 
      }

      if (!done) {
        prf.push_back(*p0);
      }
    }

    if (prf == prfbefore) {
      break;
    }

    std::swap(prf, prfbefore);
    prf.clear();
  }

  paths.insert(prfbefore.begin(), prfbefore.end());

  return std::make_pair(paths, lmin);
}

MultiBNDM multiBNDMPrep(const NFA& graph, const std::vector<StateLayoutInfo>& snippets) {
  std::set<std::tuple<NFA::VertexDescriptor,uint32_t,std::vector<ByteSet>>> paths;
  size_t lmin;

  std::tie(paths, lmin) = prefixPrep(graph);

  const size_t dwidth = (paths.size()*lmin+63)/64;
  std::unique_ptr<uint64_t[]> b(new uint64_t[256*dwidth]());
  std::vector<std::pair<uint32_t,uint32_t>> pcmap(paths.size(), {0, NOLABEL});

  size_t pos = 0;
  for (const auto& p: paths) {
    pcmap[pos/lmin] = { snippets[std::get<0>(p)].Start, std::get<1>(p) };
    for (auto cc = std::get<2>(p).crbegin(); cc != std::get<2>(p).crend(); ++cc) {
      for (int c = 0; c < 256; ++c) {
        if ((*cc)[c]) {
          b[c*dwidth+pos/64] |= (1ul << (pos % 64));
        }
      }
      ++pos;
    }
  }

  for (int i = 0; i < 256; ++i) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << i << ' ';
    for (int j = dwidth-1; j >= 0; --j) {
      std::cerr << std::bitset<64>(b[dwidth*i+j]);
    }
    std::cerr << '\n';
  }
  std::cerr << std::dec << '\n';

  for (int i = 0; i < pcmap.size(); ++i) {
    std::cerr << std::setw(3)  << std::setfill(' ') << i << ' '
              << std::setw(5)  << std::setfill(' ') << pcmap[i].first << ' '
              << std::setw(10) << std::setfill(' ') << pcmap[i].second << '\n';
  }
  std::cerr << std::endl;

  return MultiBNDM(b.get(), lmin, dwidth, pcmap);
}

std::pair<uint32_t,std::bitset<256*256>> bestPair(const NFA& graph) {
  std::set<std::pair<uint32_t,NFA::VertexDescriptor>> next;
  next.emplace(0, 0);

  std::vector<std::bitset<256*256>> b;

  uint32_t lmin = std::numeric_limits<uint32_t>::max()-1;

  uint32_t depth;
  NFA::VertexDescriptor h;

  while (!next.empty()) {
    std::tie(depth, h) = *next.begin();
    next.erase(next.begin());

    // lmin + 1 still gets us one transition followed by any byte;
    // > lmin + 1 would have to accept everytyhing, so is useless
    if (depth > lmin + 1) {
      break;
    }

    if (graph[h].IsMatch && depth < lmin) {
      lmin = depth;
    }

    // put successors in the queue if they're at lmin + 1 or less
    if (depth < lmin + 1) {
      for (const NFA::VertexDescriptor t : graph.outVertices(h)) {
        next.emplace(depth+1, t);
      }
    }

    // ensure that b[depth] exists
    if (b.size() <= depth) {
      b.resize(depth+1);
    }
  
    // we use this pointer for getting sub-bitsets 
    uint8_t* const bb = reinterpret_cast<uint8_t* const>(&b[depth]);
 
    for (const NFA::VertexDescriptor t0 : graph.outVertices(h)) {
      ByteSet first;
      graph[t0].Trans->orBytes(first);

      if (graph[t0].IsMatch) {
        // match; record each first byte followed by any byte
        for (uint32_t s = 0; s < 256; ++s) {
          *reinterpret_cast<std::bitset<256>*>(bb + (s << 5)) |= first;
        }
      }
      else {
        // no match; record each first byte followed by each second byte
        for (const NFA::VertexDescriptor t1 : graph.outVertices(t0)) {
          ByteSet second;
          graph[t1].Trans->orBytes(second);

          for (uint32_t s = 0; s < 256; ++s) {
            if (second.test(s)) {
              *reinterpret_cast<std::bitset<256>*>(bb + (s << 5)) |= first;
            }
          }
        }
      }
    }
  }

  if (b.size() < lmin) {
    // Don't go beyond the end of the vector. This can happen with some
    // tests where we have no match states.
    lmin = b.size();
  }
  else if (lmin < b.size()) {
    // Don't start pairs after offset lmin-1, as [lmin,lmin+2) is
    // guaranteed to be informationless.
    b.resize(lmin);
  }

  // Return the offset and bitset for the best two-byte window
  const auto i = std::min_element(b.begin(), b.end(),
    [](const std::bitset<256*256>& l, const std::bitset<256*256>& r) {
      return l.count() < r.count();
    }
  );

  return {i-b.begin(), *i};
}

std::vector<std::vector<NFA::VertexDescriptor>> pivotStates(NFA::VertexDescriptor source, const NFA& graph) {
  std::vector<std::vector<NFA::VertexDescriptor>> ret(256);
  ByteSet permitted;

  for (const NFA::VertexDescriptor ov : graph.outVertices(source)) {
    graph[ov].Trans->getBytes(permitted);
    for (uint32_t i = 0; i < 256; ++i) {
      if (permitted[i] && std::find(ret[i].begin(), ret[i].end(), ov) == ret[i].end()) {
        ret[i].push_back(ov);
      }
    }
  }
  return ret;
}

uint32_t maxOutbound(const std::vector<std::vector<NFA::VertexDescriptor>>& tranTable) {
  return std::max_element(tranTable.begin(), tranTable.end(),
    [](const std::vector<NFA::VertexDescriptor>& l,
       const std::vector<NFA::VertexDescriptor>& r) {
      return l.size() < r.size();
    }
  )->size();
}

void writeVertex(std::ostream& out, NFA::VertexDescriptor v, const NFA& graph) {
  out << "  " << v << " [label=\"" << v << "\"";

  if (graph[v].IsMatch) {
    // double ring for match states
    out << ", peripheries=2";
  }

  out << "];\n";
}

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

void writeEdge(std::ostream& out, NFA::VertexDescriptor v, NFA::VertexDescriptor u, uint32_t priority, const NFA& graph) {
  const std::string esclabel = escape('"', escape('\\', graph[u].label()));

  out << "  " << v << " -> " << u << " ["
      << "label=\"" << esclabel << "\", "
      << "taillabel=\"" << priority << "\"];\n";
}

void writeGraphviz(std::ostream& out, const NFA& graph) {
  out << "digraph G {\n  rankdir=LR;\n  ranksep=equally;\n  node [shape=\"circle\"];" << std::endl;

  for (const NFA::VertexDescriptor v : graph.vertices()) {
    writeVertex(out, v, graph);
  }

  for (const NFA::VertexDescriptor head : graph.vertices()) {
    for (uint32_t j = 0; j < graph.outDegree(head); ++j) {
      writeEdge(out, head, graph.outVertex(head, j), j, graph);
    }
  }

  out << "}" << std::endl;
}
