#pragma once

#include <bitset>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "instructions.h"
#include "thread.h"

class MultiBNDM {
public:
  MultiBNDM() {}

  MultiBNDM(const MultiBNDM& other):
    Lmin(other.Lmin), Dwidth(other.Dwidth),
    B(new uint64_t[256*Dwidth]),
    D(new uint64_t[Dwidth]),
    CL(new uint64_t[Dwidth]),
    DF(new uint64_t[Dwidth]),
    PCMap(other.PCMap)
  {
    std::copy(other.B.get(), other.B.get() + 256*Dwidth, B.get());
    std::copy(other.D.get(), other.D.get() + Dwidth, D.get());
    std::copy(other.CL.get(), other.CL.get() + Dwidth, CL.get());
    std::copy(other.DF.get(), other.DF.get() + Dwidth, DF.get());
  }

  MultiBNDM& operator=(const MultiBNDM& other) {
    Lmin = other.Lmin;
    Dwidth = other.Dwidth;
     
    B.reset(new uint64_t[256*Dwidth]);
    D.reset(new uint64_t[Dwidth]);
    CL.reset(new uint64_t[Dwidth]);
    DF.reset(new uint64_t[Dwidth]);

    std::copy(other.B.get(), other.B.get() + 256*Dwidth, B.get());
    std::copy(other.D.get(), other.D.get() + Dwidth, D.get());
    std::copy(other.CL.get(), other.CL.get() + Dwidth, CL.get());
    std::copy(other.DF.get(), other.DF.get() + Dwidth, DF.get());

    PCMap = other.PCMap;

    return *this;
  }

  MultiBNDM(MultiBNDM&&) = default;

  MultiBNDM& operator=(MultiBNDM&&) = default;

  MultiBNDM(const uint64_t* const b, size_t lmin, size_t dwidth, const std::vector<std::pair<uint32_t,uint32_t>>& pcmap):
    Lmin(lmin), Dwidth(dwidth),
    B(new uint64_t[256*Dwidth]),
    D(new uint64_t[Dwidth]()),
    CL(new uint64_t[Dwidth]()),
    DF(new uint64_t[Dwidth]()),
    PCMap(pcmap)
  {
    std::copy(b, b + 256*Dwidth, B.get());

    const uint64_t clmask = (1ul << Lmin) - 2;
    const uint64_t dfmask = 1ul << (Lmin - 1); 

    for (size_t i = 0; i < Dwidth*64; i += Lmin) {
      CL[i/64] |= clmask << (i % 64);
      DF[i/64] |= dfmask << (i % 64);

      if (i/64+1 < Dwidth && 64 - Lmin < i % 64) {
        CL[i/64+1] |= (clmask >> (64 - i % 64));
        DF[i/64+1] |= (dfmask >> (64 - i % 64));
      }
    }

    std::cerr << "CL == ";
    for (int j = Dwidth-1; j >= 0; --j) {
      std::cerr << std::bitset<64>(CL[j]);
    }
    std::cerr << '\n';

    std::cerr << "DF == ";
    for (int j = Dwidth-1; j >= 0; --j) {
      std::cerr << std::bitset<64>(DF[j]);
    }
    std::cerr << '\n';
  }

  uint32_t search(const Instruction* const base, const byte* const cur, const uint64_t offset, std::vector<Thread>& /* tlist */, std::function<void(const Instruction* const pc, const uint32_t label, const uint64_t offset)> createThread) {
    std::fill(D.get(), D.get() + Dwidth, ~0ul);

    uint32_t last = Lmin, j = Lmin;
    bool any;
    do {
      const uint64_t* const bcur = B.get() + (*(cur+j-1))*Dwidth;
      for (size_t i = 0; i < Dwidth; ++i) {
        D[i] &= bcur[i];
      }
      --j;

      for (size_t i = 0; i < Dwidth; ++i) {
        if (D[i] & DF[i]) {
          if (j > 0) {
            last = j;
            break;
          }
          else {
            std::cerr << "D[" << i << "] & DF[" << i << "], j == " << j << std::endl;

            // match
            // TODO: don't bother looking at D[x] for x < i
            for (i = Lmin - 1; i < Dwidth*64; i += Lmin) {
              if (D[i/64] & (1ul << (i % 64))) {
/*
                tlist.emplace_back(
                  base + PCMap[i/Lmin].first,
                  PCMap[i/Lmin].second,
                  #ifdef LBT_TRACE_ENABLED
                  NextId++,
                  #endif
                  offset, Thread::NONE
                );
//                std::cerr << base << ' ' << tlist.back() << std::endl; 
    
                #ifdef LBT_TRACE_ENABLED
                new_thread_json.insert(tlist.back().Id);
                #endif
*/

                createThread(
                  base + PCMap[i/Lmin].first,
                  PCMap[i/Lmin].second,
                  offset
                );
              }
            }

            // shift
            for (int k = Dwidth - 1; k > 0; --k) {
              D[k] = ((D[k] << 1) | (D[k-1] >> 63)) & CL[k];
            }
            D[0] = (D[0] << 1) & CL[0];

            return 0;
          }
        }
      }

      any = false;
      for (int i = Dwidth - 1; i > 0; --i) {
        any |= (D[i] = ((D[i] << 1) | (D[i-1] >> 63)) & CL[i]);
      }
      any |= (D[0] = (D[0] << 1) & CL[0]);
    } while (any);

    return last - 1;
  }

  size_t width() const { return Lmin; }

private:
  size_t Lmin, Dwidth;
  std::unique_ptr<uint64_t[]> B, D, CL, DF;
  std::vector<std::pair<uint32_t,uint32_t>> PCMap;
};

/*
class TwoByte {
public:
  uint32_t search(const byte* const cur, const uint64_t offset, std::vector<Thread>& tlist) {
// FIXME: The ifdefs here are now broken 
    if (Filter[*reinterpret_cast<const uint16_t* const>(cur+FilterOff)]) {
      for (const Instruction* pc: First) {
// FIXME: would rather call Vm::_createThread here
        tlist.emplace_back(
          pc, Thread::NOLABEL,
          #ifdef LBT_TRACE_ENABLED
          NextId++,
          #endif
          offset, Thread::NONE
        );
    
        #ifdef LBT_TRACE_ENABLED
        new_thread_json.insert(Active.back().Id);
        #endif
      }
    }

    return 0;
  }

  size_t width() const { return FilterOff + 2; }

  std::bitset<256*256> Filter;
  size_t FilterOff;
  std::vector<const Instruction*> First;
};
*/
