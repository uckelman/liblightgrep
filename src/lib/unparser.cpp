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

#include "unparser.h"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <ostream>

#include <boost/lexical_cast.hpp>

bool is_binary(const ParseNode* n) {
  return n->Type == ParseNode::ALTERNATION || n->Type == ParseNode::CONCATENATION;
}

bool is_atomic(const ParseNode* n) {
  return n->Type == ParseNode::DOT || n->Type == ParseNode::CHAR_CLASS
                                   || n->Type == ParseNode::LITERAL;
}

//
// Parentheses are necessary when:
//
// * a unary operator is the parent of a binary operator
// * concatenation is the parent of an alternation
// * a repetition operator is the parent of another repetition operator
//

void open_paren(std::ostream& out, const ParseNode* n) {
  if (!is_binary(n) && !is_atomic(n->Child.Left)) {
    out << '(';
  }
}

void close_paren(std::ostream& out, const ParseNode* n) {
  if (!is_binary(n) && !is_atomic(n->Child.Left)) {
    out << ')';
  }
}

std::string byteToLiteralString(uint32_t i) {
  // all the characters fit to print unescaped
  if (i == '\\') {
    return "\\\\";
  }
  else if (0x20 <= i && i <= 0x7E) {
    return std::string(1, (char) i);
  }
  else {
    switch (i) {
    // all of the named single-character escapes
    case 0x07: return "\\a";
    case 0x09: return "\\t";
    case 0x0A: return "\\n";
    case 0x0C: return "\\f";
    case 0x0D: return "\\r";
    case 0x1B: return "\\e";
    // otherwise, print the hex code
    default:
      {
        std::ostringstream ss;
        ss << "\\x" << std::hex << std::uppercase
                    << std::setfill('0') << std::setw(2) << i;
        return ss.str();
      }
    }
  }
}

std::string byteToCharacterString(uint32_t i) {
  // all the characters fit to print unescaped
  if (i == '\\') {
    return "\\\\";
  }
  else if (0x20 <= i && i <= 0x7E) {
    return std::string(1, (char) i);
  }
  else {
    switch (i) {
    // all of the named single-character escapes
    case 0x07: return "\\a";
    case 0x08: return "\\b";
    case 0x09: return "\\t";
    case 0x0A: return "\\n";
    case 0x0C: return "\\f";
    case 0x0D: return "\\r";
    case 0x1B: return "\\e";
    // otherwise, print the hex code
    default:
      {
        std::ostringstream ss;
        ss << "\\x" << std::hex << std::uppercase
                    << std::setfill('0') << std::setw(2) << i;
        return ss.str();
      }
    }
  }
}

/*
 * Rules for escaping inside character classes:
 *
 * ']' must be escaped unless it is first, or immediately follows a negation
 * '^' must be escaped if it is first
 * '-' must be escaped if it would form an unwanted range
 * '\' must be escaped
 *
 */

std::string byteSetToCharacterClass(const ByteSet& bs) {

  // check relative size of 0 and 1 ranges
  int sizediff = -1; // negated has a 1-char disadvantage due to the '^'
  uint32_t left = 0;

  bool hasBoth = false;

  for (uint32_t i = 1; i < 257; ++i) {
    if (i < 256 && bs[i] ^ bs[0]) {
      hasBoth = true;
    }

    if (i == 256 || bs[i-1] ^ bs[i]) {
      const int len = std::min(i - left, (uint32_t) 3);
      sizediff += bs[i-1] ? len : -len;
      left = i;
    }
  }

  // is this a full or empty character class?
  if (!hasBoth) {
    return bs[0] ? "\\z00-\\zFF" : "^\\z00-\\zFF";
  }

  // will char class will be shorter if negated?
  const bool invert = sizediff > 0;

  std::ostringstream ss;

  if (invert) {
    ss << '^';
  }

  bool first = true;
  bool caret = false;
  bool hyphen = false;

  left = 256;

  for (uint32_t i = 0; i < 257; ++i) {
    if (i < 256 && (invert ^ bs[i])) {
      if (left > 0xFF) {
        // start a new range
        left = i;
      }
    }
    else if (left <= 0xFF) {
      // write a completed range
      uint32_t right = i-1;

      // shrink ranges so that the hyphen is neither endpoint
      if (left == '-') {
        hyphen = true;

        if (left == right) {
          // hyphen is the whole range
          left = 256;
          first = false;
          continue;
        }

        ++left;
      }
      else if (right == '-') {
        hyphen = true;
        --right;
      }

      // shrink initial range so that the caret is not the start
      if ((first || hyphen) && left == '^') {
        caret = true;

        if (left == right) {
          // caret is the whole range
          left = 256;
          first = false;
          continue;
        }

        ++left;
      }

      if (right - left + 1 < 4) {
        // enumerate small ranges
        for (uint32_t j = left; j <= right; ++j) {
          if (j == ']') {
            if (first) {
              first = false;
            }
            else {
              ss << '\\';
            }
          }

          ss << byteToCharacterString(j);
        }
      }
      else {
        // use '-' for large ranges

        if (left == ']') {
          if (first) {
            first = false;
          }
          else {
            ss << '\\';
          }
        }

        ss << byteToCharacterString(left) << '-';

        if (right == ']') {
          ss << '\\';
        }

        ss << byteToCharacterString(right);
      }

      left = 256;
      first = false;
    }
  }

  // if there was a hyphen, put it at the end
  if (hyphen) {
    if (caret) {
      // if we haven't written anything, reverse the hyphen and caret
      if (0 == ss.tellp()) {
        ss << byteToCharacterString('-') << byteToCharacterString('^');
      }
      else {
        ss << byteToCharacterString('^') << byteToCharacterString('-');
      }
    }
    else {
      ss << byteToCharacterString('-');
    }
  }
  // if there was a caret, put it at the end
  else if (caret) {
    // if we haven't written anything, escape the caret
    if (0 == ss.tellp()) {
      ss << '\\';
    }
    ss << byteToCharacterString('^');
  }

  return ss.str();
}

void unparse(std::ostream& out, const ParseNode* n) {
  switch (n->Type) {
  case ParseNode::REGEXP:
    if (!n->Child.Left) {
      return;
    }

    unparse(out, n->Child.Left);
    break;

  case ParseNode::LOOKBEHIND_POS:
    out << "(?<=";
    unparse(out, n->Child.Left);
    out << ')';
    break;

  case ParseNode::LOOKBEHIND_NEG:
    out << "(?<!";
    unparse(out, n->Child.Left);
    out << ')';
    break;

  case ParseNode::LOOKAHEAD_POS:
    out << "(?=";
    unparse(out, n->Child.Left);
    out << ')';
    break;

  case ParseNode::LOOKAHEAD_NEG:
    out << "(?!";
    unparse(out, n->Child.Left);
    out << ')';
    break;

  case ParseNode::ALTERNATION:
    unparse(out, n->Child.Left);
    out << '|';
    unparse(out, n->Child.Right);
    break;

  case ParseNode::CONCATENATION:
    if (n->Child.Left->Type == ParseNode::ALTERNATION) {
      out << '(';
      unparse(out, n->Child.Left);
      out << ')';
    }
    else {
      unparse(out, n->Child.Left);
    }

    if (n->Child.Right->Type == ParseNode::ALTERNATION) {
      out << '(';
      unparse(out, n->Child.Right);
      out << ')';
    }
    else {
      unparse(out, n->Child.Right);
    }
    break;

  case ParseNode::REPETITION:
    open_paren(out, n);
    unparse(out, n->Child.Left);
    close_paren(out, n);
    repetition(out, n->Child.Rep.Min, n->Child.Rep.Max);
    break;

  case ParseNode::REPETITION_NG:
    open_paren(out, n);
    unparse(out, n->Child.Left);
    close_paren(out, n);
    repetition(out, n->Child.Rep.Min, n->Child.Rep.Max);
    out << '?';
    break;

  case ParseNode::DOT:
    out << '.';
    break;

  case ParseNode::CHAR_CLASS:
    {
      ByteSet bs;
      for (uint32_t i = 0; i < 256; ++i) {
        bs.set(i, n->Set.CodePoints.test(i));
      }

      out << '[' << byteSetToCharacterClass(bs) << ']';
    }
    break;

  case ParseNode::LITERAL:
    out << byteToLiteralString(n->Val);
    break;

  case ParseNode::BYTE:
    out << byteToLiteralString(n->Val);
    break;

  default:
    // WTF?
    throw std::logic_error(boost::lexical_cast<std::string>(n->Type));
  }
}

std::string unparse(const ParseTree& tree) {
  std::ostringstream ss;
  unparse(ss, tree.Root);
  return ss.str();
}

