#include <algorithm>
#include <bitset>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

#include "byteset.h"

void print_row(const uint64_t* const r, const uint32_t w) {
  for (uint32_t i = 0; i < w; ++i) {
    std::cerr << std::bitset<64>(r[i]);
  }
}

std::unique_ptr<uint64_t[]> shift_and_prep(const char* const beg, const char* const end) {
  const size_t bwidth = std::ceil((end-beg)/64.0);
  const size_t bsize = 256*bwidth;
  std::unique_ptr<uint64_t[]> b(new uint64_t[bsize]);
  std::fill(b.get(), b.get() + bsize, 0);

  for (const char* cur = beg; cur < end; ++cur) {
    b[(*cur)*bwidth + (cur-beg)/64] |= (1ul << ((cur-beg) % 64));
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

std::unique_ptr<uint64_t[]> shift_and_cc_prep(const char* const beg, const char* const end) {
  const size_t bwidth = std::ceil((end-beg)/64.0);
  const size_t bsize = 256*bwidth;
  std::unique_ptr<uint64_t[]> b(new uint64_t[bsize]);
  std::fill(b.get(), b.get() + bsize, 0);

  for (const char* cur = beg; cur < end; ++cur) {
    b[std::tolower(*cur)*bwidth + (cur-beg)/64] |= (1ul << ((cur-beg) % 64));
    b[std::toupper(*cur)*bwidth + (cur-beg)/64] |= (1ul << ((cur-beg) % 64));
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

void shift_and_search(const uint64_t* const b, const uint32_t m, const char* const beg, const char* const end) {
  const size_t dlen = std::ceil(m/64.0);
  const size_t drem = (m - 1) % 64;
  std::unique_ptr<uint64_t[]> d(new uint64_t[dlen]);
  std::fill(d.get(), d.get() + dlen, 0);

  for (const char* cur = beg; cur < end; ++cur) {
    // D = ((D << 1) | 1) & B[*cur]
    const uint64_t* const bcur = b + dlen*static_cast<uint8_t>(*cur);
    for (int i = dlen - 1; i > 0; --i) {
      d[i] = ((d[i] << 1) | (d[i-1] >> 63)) & bcur[i];
    }
    d[0] = ((d[0] << 1) | 1) & bcur[0];

    if (d[dlen-1] & (1ul << drem)) {
//      std::cout << (cur - beg - m + 1) << '\n';
    }
  }
}

std::unique_ptr<uint64_t[]> shift_or_prep(const char* const beg, const char* const end) {
  std::unique_ptr<uint64_t[]> b(shift_and_prep(beg, end));

  for (int c = 0; c < 256; ++c) {
    b[c] = ~b[c];
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

std::unique_ptr<uint64_t[]> shift_or_cc_prep(const char* const beg, const char* const end) {
  std::unique_ptr<uint64_t[]> b(shift_and_cc_prep(beg, end));

  for (int c = 0; c < 256; ++c) {
    b[c] = ~b[c];
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

uint32_t shift_or_search(const uint64_t* const b, const uint32_t m, const char* const beg, const char* const end) {
  const size_t dlen = std::ceil(m/64.0);
  const size_t drem = (m - 1) % 64;
  std::unique_ptr<uint64_t[]> d(new uint64_t[dlen]);
  std::fill(d.get(), d.get() + dlen, ~0ul);

  uint32_t count = 0;

  for (const char* cur = beg; cur < end; ++cur) {
    // D = (D << 1) | B[*cur]
    const uint64_t* const bcur = b + dlen*static_cast<uint8_t>(*cur);
    for (int i = dlen - 1; i > 0; --i) {
      d[i] = ((d[i] << 1) | (d[i-1] >> 63)) | bcur[i];
    }
    d[0] = (d[0] << 1) | bcur[0];

    if (~d[dlen-1] & (1ul << drem)) {
//      std::cout << (cur - beg - m + 1) << '\n';
      ++count;
    }
  }

  return count;
}

std::unique_ptr<uint64_t[]> bndm_prep(const char* const beg, const char* const end) {
  const size_t bwidth = std::ceil((end-beg)/64.0);
  const size_t bsize = 256*bwidth;
  std::unique_ptr<uint64_t[]> b(new uint64_t[bsize]);
  std::fill(b.get(), b.get() + bsize, 0);

  for (const char* cur = beg; cur < end; ++cur) {
    b[(*cur)*bwidth + (cur-beg)/64] |= (1ul << ((end-beg) - ((cur-beg) % 64) - 1));
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

uint32_t bndm_search(const uint64_t* const b, const uint32_t m, const char* const beg, const char* const end) {
  uint32_t count = 0;

  const size_t dlen = std::ceil(m/64.0);
  const size_t drem = (m - 1) % 64;
  std::unique_ptr<uint64_t[]> d(new uint64_t[dlen]);

  uint32_t last, j;
  for (const char* cur = beg; cur < end - m; cur += last) {
    last = j = m;
    std::fill(d.get(), d.get() + dlen, ~0ul);

    bool any;
    do {
      const uint64_t* const bcur = b + dlen*static_cast<uint8_t>(*(cur+j-1));
      for (uint32_t i = 0; i < dlen; ++i) {
        d[i] &= bcur[i];
      }

      --j;
      
      if (d[dlen-1] & (1ul << drem)) {
        if (j > 0) {
          last = j;
        }
        else {
//          std::cout << (cur - beg) << '\n';
          ++count;
        }
      }
      
      any = false;
      for (int i = dlen - 1; i > 0; --i) {
        any |= (d[i] = (d[i] << 1) | (d[i-1] >> 63));
      }
      any |= (d[0] <<= 1);
    } while (any);
  }

  return count;
}

std::unique_ptr<uint64_t[]> bndm_cc_prep(const char* const beg, const char* const end) {
  const size_t bwidth = std::ceil((end-beg)/64.0);
  const size_t bsize = 256*bwidth;
  std::unique_ptr<uint64_t[]> b(new uint64_t[bsize]);
  std::fill(b.get(), b.get() + bsize, 0);

  for (const char* cur = beg; cur < end; ++cur) {
    b[std::tolower(*cur)*bwidth + (cur-beg)/64] |= (1ul << ((end-beg) - ((cur-beg) % 64) - 1));
    b[std::toupper(*cur)*bwidth + (cur-beg)/64] |= (1ul << ((end-beg) - ((cur-beg) % 64) - 1));
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

std::unique_ptr<uint64_t[]> bndm_twain_prep(const char* const, const char* const) {
  
  const size_t bwidth = 1;
  const size_t bsize = 256*bwidth;
  std::unique_ptr<uint64_t[]> b(new uint64_t[bsize]);
  std::fill(b.get(), b.get() + bsize, 0);

  b['\n'*bwidth] |= (1ul << 2);
  b['\r'*bwidth] |= (1ul << 2);

  for (char c = 'a'; c <= 'z'; ++c) {
    b[c*bwidth] |= (1ul << 2);
    b[c*bwidth] |= (1ul << 0);
  }

  for (uint8_t c = 0; c < 0x80; ++c) {
    b[c*bwidth] |= (1ul << 1);
  }

  for (char c = 'A'; c <= 'Z'; ++c) {
    b[c*bwidth] |= (1ul << 2);
    b[c*bwidth] |= (1ul << 0);
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

std::unique_ptr<uint64_t[]> bndm_be_prep(const char* const, const char* const) {

  const size_t bwidth = 1;
  const size_t bsize = 256*bwidth;
  std::unique_ptr<uint64_t[]> b(new uint64_t[bsize]);
  std::fill(b.get(), b.get() + bsize, 0);

  const std::bitset<256> at[] = {
    std::bitset<256>("1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000000000000000000000000000000000000000001000000000111111111111111111111111110100001111111111111111111111111100000001111111111011010010010001000000000000000000000000000000011"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000100000000000000000000000000000000000001000010000111111111111111111111111110100001111111111111111111111111100000001111111111011011010010000100000000000000000010010000000001"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000100000000000000000000000000000000000001000010000111111111111111111111111110100001111111111111111111111111110000011111111111111110000010001100000000000000000010011000000011"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000100000000000000000000000000000000000001000010000111111111111111111111111110100001111111111111111111111111110000011111111111111111000010000100000000000000000010011000000011"),
    std::bitset<256>("0000000000011111111111111111111111111111111111111111111111111100100000000000000000000100000000000000000000000000000000000001000011111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111")
  };

  for (int i = 0; i < 6; ++i) {
    std::cerr << reinterpret_cast<const ByteSet&>(at[i]) << '\n'; 
    for (int c = 0; c < 256; ++c) {
      if (at[i][c]) {
        b[c*bwidth] |= (1ul << (5-i));
      }
    }
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

std::unique_ptr<uint64_t[]> shift_or_be_prep(const char* const, const char* const) {
  const size_t bwidth = 1;
  const size_t bsize = 256*bwidth;
  std::unique_ptr<uint64_t[]> b(new uint64_t[bsize]);
  std::fill(b.get(), b.get() + bsize, 0);

  const std::bitset<256> at[] = {
    std::bitset<256>("1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000000000000000000000000000000000000000001000000000111111111111111111111111110100001111111111111111111111111100000001111111111011010010010001000000000000000000000000000000011"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000100000000000000000000000000000000000001000010000111111111111111111111111110100001111111111111111111111111100000001111111111011011010010000100000000000000000010010000000001"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000100000000000000000000000000000000000001000010000111111111111111111111111110100001111111111111111111111111110000011111111111111110000010001100000000000000000010011000000011"),
    std::bitset<256>("0000000000000000000000000000010000000000000000000000000000100000100000000000000000000100000000000000000000000000000000000001000010000111111111111111111111111110100001111111111111111111111111110000011111111111111111000010000100000000000000000010011000000011"),
    std::bitset<256>("0000000000011111111111111111111111111111111111111111111111111100100000000000000000000100000000000000000000000000000000000001000011111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111")
  };

  for (int i = 0; i < 6; ++i) {
    for (int c = 0; c < 256; ++c) {
      if (at[i][c]) {
        b[c*bwidth] |= (1ul << i);
      }
    }
  } 

  for (int c = 0; c < 256; ++c) {
    b[c] = ~b[c];
  }

/*
  for (int c = 0; c < 256; ++c) {
    std::cerr << std::hex << std::setw(2) << std::setfill('0') << c << ' ';
    print_row(b.get() + bwidth*c, bwidth);
    std::cerr << '\n';
  }
*/

  return std::move(b);
}

template <
  std::unique_ptr<uint64_t[]> (*Prep)(const char* const, const char* const),
  uint32_t (*Search)(const uint64_t* const, const uint32_t, const char* const, const char* const)
>
void harness(const std::string& pat, const std::vector<char>& text) {
  const std::unique_ptr<const uint64_t[]> b(
    Prep(pat.c_str(), pat.c_str() + pat.size())
  );

  const auto beg = std::chrono::high_resolution_clock::now();
  const uint32_t count =
    Search(b.get(), pat.size(), text.data(), text.data() + text.size());
  const auto end = std::chrono::high_resolution_clock::now();

  const std::chrono::duration<double> elapsed = end - beg;
  std::cerr << ((double) text.size() / (1 << 20)) << "MB, "
            << elapsed.count() << "s, " 
            << ((text.size() / elapsed.count()) / (1 << 20)) << "MB/s, "
            << count << " hits, "
            << ((double) count / text.size()) << '\n';
}

int main(int, char** argc) {
  const std::string pat(argc[1]);

  const std::vector<char> text(
    std::istream_iterator<char>(std::cin),
    std::istream_iterator<char>()
  );

/*
  harness<shift_and_prep, shift_and_search>(pat, text);
  harness<shift_or_prep,  shift_or_search >(pat, text);
  harness<bndm_prep,      bndm_search     >(pat, text);

  harness<shift_and_cc_prep, shift_and_search>(pat, text);
  harness<shift_or_cc_prep,  shift_or_search >(pat, text);
  harness<bndm_cc_prep,      bndm_search     >(pat, text);
*/

//  harness<bndm_twain_prep, bndm_search>(pat, text);
  harness<shift_or_be_prep, shift_or_search>(pat, text);
//  harness<bndm_be_prep,     bndm_search    >(pat, text);

  return 0;
}
