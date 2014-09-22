#pragma once

#include "byteset.h"

#include <algorithm>
#include <bitset>
#include <iomanip>
#include <iostream>

class BestTwoByte {
public:
  BestTwoByte(uint32_t off, std::bitset<256*256> bits):
    Offset(off), TwoBytes(bits) {}

  size_t skip(const byte* const pos) const {
    return !TwoBytes[*reinterpret_cast<const uint16_t* const>(pos+Offset)];
  }

  size_t width() const { return Offset + 2; }

private:
  uint32_t Offset;
  std::bitset<256*256> TwoBytes;
};

class ShiftOr {
public:
  ShiftOr(const std::vector<ByteSet>& pat):
    M(pat.size()), B{}, D(~0)
  {
    for (size_t i = 0; i < pat.size(); ++i) {
      const uint64_t mask = 1 << i;
      for (int c = 0; c < 256; ++c) {
        if (pat[i][c]) {
          B[c] |= mask;
        }
      }
    }

    for (auto& w: B) { w = ~w; }

//    for (int i = 0; i < 256; ++i) {
//      std::cerr << std::hex << std::setw(2) << std::setfill('0') << i
//                << ' ' << std::bitset<64>(B[i]) << '\n';
//    }
  }

  size_t skip(const byte* const cur) {
    D = (D << 1) | B[*(cur+M-1)];
    return (~D & (1 << (M-1))) ? 0 : 1;
  }

  size_t width() const { return M; }

private:
  size_t M;
  uint8_t B[256], D;
};

/*
class ShiftOr {
public:
  ShiftOr(const std::vector<ByteSet>& pat):
    M(pat.size()), B{}, D(~0ul)
  {
    for (size_t i = 0; i < pat.size(); ++i) {
      const uint64_t mask = 1ul << i;
      for (int c = 0; c < 256; ++c) {
        if (pat[i][c]) {
          B[c] |= mask;
        }
      }
    }

    for (auto& w: B) { w = ~w; }

//    for (int i = 0; i < 256; ++i) {
//      std::cerr << std::hex << std::setw(2) << std::setfill('0') << i
//                << ' ' << std::bitset<64>(B[i]) << '\n';
//    }
  }

  size_t skip(const byte* const cur) {
    D = (D << 1) | B[*(cur+M-1)];
    return (~D & (1ul << (M-1))) ? 0 : 1;
  }

  size_t width() const { return M; }

private:
  size_t M; 
  uint64_t B[256], D;
};
*/

/*
class ShiftOr {
public:
  ShiftOr(const std::vector<ByteSet>& pat):
    M(pat.size()), DLen(std::ceil(M/64.0)), DRem((M-1) % 64),
    B(new uint64_t[256*DLen]), D(new uint64_t[DLen])
  {
    std::fill(D.get(), D.get() + DLen, ~0ul);
    std::fill(B.get(), B.get() + 256*DLen, 0);

    for (size_t i = 0; i < pat.size(); ++i) {
      for (int c = 0; c < 256; ++c) {
        if (pat[i][c]) {
          B[c*DLen + i/64] |= (1ul << (i % 64));
        }
      }
    }

    std::transform(B.get(), B.get() + 256*DLen, B.get(), [](uint64_t w) { return ~w; });
  }

  size_t skip(const byte* const cur) const {
    // D = (D << 1) | B[*cur]
    const uint64_t* const bcur = B.get() + DLen*static_cast<uint8_t>(*cur);
    for (int i = DLen - 1; i > 0; --i) {
      D[i] = ((D[i] << 1) | (D[i-1] >> 63)) | bcur[i];
    }
    D[0] = (D[0] << 1) | bcur[0];
  
    return !(~D[DLen-1] & (1ul << DRem));
  }

  size_t width() const { return M; }

private:
  size_t M, DLen, DRem; 
  std::unique_ptr<uint64_t[]> B, D;
};
*/

class BNDM {
public:
  BNDM(const std::vector<ByteSet>& pat):
    M(pat.size()), B{}, D(~0)
  {
    for (size_t i = 0; i < pat.size(); ++i) {
      const uint64_t mask = 1 << (M - i - 1);
      for (int c = 0; c < 256; ++c) {
        if (pat[i][c]) {
          B[c] |= mask;
        }
      }
    }

//    for (int i = 0; i < 256; ++i) {
//      std::cerr << std::hex << std::setw(2) << std::setfill('0') << i
//                << ' ' << std::bitset<64>(B[i]) << '\n';
//    }
  }

  size_t skip(const byte* const cur) {
    uint32_t last = M, j = M;
    do {
      D &= B[*(cur+j-1)];
      --j;

      if (D & (1 << (M-1))) {
        if (j > 0) {
          last = j;
        }
        else {
          D <<= 1;
          return 0;
        }
      }
      
      D <<= 1;
    } while (D);

    return last;
  }

  size_t width() const { return M; }

private:
  size_t M;
  uint8_t B[256], D;
};
