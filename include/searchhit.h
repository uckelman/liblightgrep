#pragma once

#include "basic.h"
#include "lightgrep_search_hit.h"

class SearchHit: public LG_SearchHit {
public:
  SearchHit() {}

  // note that this takes the length
  SearchHit(uint64 start, uint64 end, uint32 lbl) {
    Start = start;
    End = end;
    KeywordIndex = lbl;
  }

  uint64 length() const {
    return End - Start;
  }

  bool operator==(const SearchHit& x) const {
    return x.Start == Start && x.End == End && x.KeywordIndex == KeywordIndex;
  }

  bool operator<(const SearchHit& x) const {
    return Start < x.Start ||
          (Start == x.Start &&
          (End < x.End ||
          (End == x.End && KeywordIndex < x.KeywordIndex)));
  }
};

template <class OutStream>
OutStream& operator<<(OutStream& out, const SearchHit& hit) {
  out << '(' << hit.Start << ", " << hit.End << ", " << hit.KeywordIndex << ')';
  return out;
}

typedef LG_HITCALLBACK_FN HitCallback;
