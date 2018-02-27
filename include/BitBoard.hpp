#ifndef BITBOARD_HPP
# define BITBOARD_HPP

# include <iostream>
# include <cstdlib>
# include <array>
/* includes for debug purposes */
# include <bitset>
# include <string>
# include <sstream>

# define N 6
# define D 8
# define BITS 64

/*  Implementation of a bitboard representation of a square board of 19*19 using
    6 long integers (64bits), and bit operations.
*/
class BitBoard {

public:
    BitBoard(void);
    BitBoard(std::array<uint64_t, N> values);
    BitBoard(BitBoard const &src);
    ~BitBoard(void);
    BitBoard	&operator=(BitBoard const &rhs);
    BitBoard	&operator=(uint64_t const &val);

    void        zeros(void);

    void        write_move(uint8_t x, uint8_t y);
    void        delete_move(uint8_t x, uint8_t y);

    void        broadcast_row(uint64_t line);
    bool        is_empty(void);

    BitBoard    opens(void) const;
    BitBoard    neighbours(void) const;

    BitBoard    shifted(uint8_t dir, uint8_t n = 1) const;
    BitBoard    dilated(void) const;
    BitBoard    eroded(void) const;

    BitBoard    rotate_315(void);
    BitBoard    rotate_45(void);

    /* arithmetic (bitwise) operator overload */
    BitBoard    operator|(BitBoard const &rhs) const; // bitwise union
    BitBoard    operator&(BitBoard const &rhs) const; // bitwise intersection
    BitBoard    operator^(BitBoard const &rhs) const; // bitwise exclusive or
    BitBoard    operator~(void) const;                // bitwise complement
    BitBoard    operator>>(int32_t shift) const;      // bitwise right shift
    BitBoard    operator<<(int32_t shift) const;      // bitwise left shift

    /* assignment operator overload */
    BitBoard    &operator|=(BitBoard const &rhs);
    BitBoard    &operator&=(BitBoard const &rhs);
    BitBoard    &operator^=(BitBoard const &rhs);
    BitBoard    &operator<<=(int32_t shift);
    BitBoard    &operator>>=(int32_t shift);

    /* member access operator overload */
    uint64_t    &operator[](uint8_t i);

    /* comparison operator overload */
    bool        operator==(BitBoard const &rhs);
    bool        operator!=(BitBoard const &rhs);

    std::array<uint64_t, N>         values;
    static std::array<int16_t, D>   shifts;
    static BitBoard                 full;
    static BitBoard                 empty;
    static BitBoard                 border_right;
    static BitBoard                 border_left;
    static BitBoard                 border_top;
    static BitBoard                 border_bottom;

};

std::ostream	&operator<<(std::ostream &os, BitBoard const &bitboard);

/* see 4.3 at https://eprints.qut.edu.au/85005/1/__staffhome.qut.edu.au_staffgroupm%24_meaton_Desktop_bits-7.pdf */
BitBoard    dilation(BitBoard const &bitboard);
BitBoard    erosion(BitBoard const &bitboard);

BitBoard    get_all_neighbours(BitBoard const &p1, BitBoard const &p2);
BitBoard    get_all_open_cells(BitBoard const &p1, BitBoard const &p2);
BitBoard    get_all_occupied_cells(BitBoard const &p1, BitBoard const &p2);

bool        detect_five_aligned(BitBoard const &bitboard);
bool        detect_gomoku_pattern_around(BitBoard const &p1, BitBoard const &p2, uint8_t const &pattern, uint8_t const &x, uint8_t const &y);
bool        detect_test(BitBoard const &bitboard);

namespace direction {
    enum direction {
        north,
        north_east,
        east,
        south_east,
        south,
        south_west,
        west,
        north_west
    };
};

#endif

/*     +--19x19 BitBoard-----------------------+
     0 | . . . . . . . . . . . . . . . . . . . | 0
     1 | . . . . . . . . . . . . . . . . . . . | 1
     2 | . . . . . . . . . . . . . . . . . . . | 2
     3 | . . . . . . ./. . . . . . . . . . . . | 3
     4 | . . . . . . . . . . . . . . . . . . . | 4
     5 | . . . . . . . . . . . . . . . . . . . | 5
     6 | . . . . . . . . . . . . . ./. . . . . | 6
     7 | . . . . . . . . . . . . . . . . . . . | 7
     8 | . . . . . . . . . . . . . . . . . . . | 8
     9 | . . . . . . . . . . . . . . . . . . . | 9
    10 | . ./. . . . . . . . . . . . . . . . . | 10
    11 | . . . . . . . . . . . . . . . . . . . | 11
    12 | . . . . . . . . . . . . . . . . . . . | 12
    13 | . . . . . . . ./. . . . . . . . . . . | 13
    14 | . . . . . . . . . . . . . . . . . . . | 14
    15 | . . . . . . . . . . . . . . . . . . . | 15
    16 | . . . . . . . . . . . . . . . ./. . . | 16
    17 | . . . . . . . . . . . . . . . . . . . | 17
    18 | . . . . . . . . . . . . . . . . . . . | 18
       +---------------------------------------+
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8

    BitBoard without separating bits. We'll benchmark the performances of the
    algorithms once we have more stuff in place, and compare the version of the
    BitBoard with and without the separator bits.

    +--Shifts---+---------------+-----+
    | direction |     value     | idx |        [indices]         [shifts]
    +-----------+---------------+-----+
    |     N     |      -19      |  0  |        7   0   1       -20 -19 -18
    |     S     |       19      |  4  |          ↖︎ ↑ ↗            ↖︎ ↑ ↗
    |     E     |        1      |  2  |        6 ← ◇ → 2       -1 ← ◇ → +1
    |     W     |       -1      |  6  |          ↙ ↓ ↘︎            ↙ ↓ ↘︎
    |    N-E    |      -18      |  1  |        5   4   3       +18 +19 +20
    |    S-E    |       20      |  3  |
    |    N-W    |      -20      |  7  |
    |    S-W    |       18      |  5  |
    +-----------+---------------+-----+

    +--Normal Board------------------+      +--Flipped Board-----------------+
    | a8  b8  c8  d8  e8  f8  g8  h8 |      | a8  a7  a6  a5  a4  a3  a2  a1 |
    | a7  b7  c7  d7  e7  f7  g7  h7 |      | b8  b7  b6  b5  b4  b3  b2  b1 |
    | a6  b6  c6  d6  e6  f6  g6  h6 |      | c8  c7  c6  c5  c4  c3  c2  c1 |
    | a5  b5  c5  d5  e5  f5  g5  h5 |      | d8  d7  d6  d5  d4  d3  d2  d1 |
    | a4  b4  c4  d4  e4  f4  g4  h4 |      | e8  e7  e6  e5  e4  e3  e2  e1 |
    | a3  b3  c3  d3  e3  f3  g3  h3 |      | f8  f7  f6  f5  f4  f3  f2  f1 |
    | a2  b2  c2  d2  e2  f2  g2  h2 |      | g8  g7  g6  g5  g4  g3  g2  g1 |
    | a1  b1  c1  d1  e1  f1  g1  h1 |      | h8  h7  h6  h5  h4  h3  h2  h1 |
    +--------------------------------+      +--------------------------------+

    Rotated BitBoards allow for efficient checks for diagonals. The examples below
    are for a chess board, but the principle applies to all sizes.
    +--Rotated Board 45--------------+      +--Rotated Board 315-------------+
    | a8 |b1  c2  d3  e4  f5  g6  h7 |      | a8  b7  c6  d5  e4  f3  g2  h1 |
    | a7  b8 |c1  d2  e3  f4  g5  h6 |      | a7  b6  c5  d4  e3  f2  g1 |h8 |
    | a6  b7  c8 |d1  e2  f3  g4  h5 |      | a6  b5  c4  d3  e2  f1 |g8  h7 |
    | a5  b6  c7  d8 |e1  f2  g3  h4 |      | a5  b4  c3  d2  e1 |f8  g7  h6 |
    | a4  b5  c6  d7  e8 |f1  g2  h3 |      | a4  b3  c2  d1 |e8  f7  g6  h5 |
    | a3  b4  c5  d6  e7  f8 |g1  h2 |      | a3  b2  c1 |d8  e7  f6  g5  h4 |
    | a2  b3  c4  d5  e6  f7  g8 |h1 |      | a2  b1 |c8  d7  e6  f5  g4  h3 |
    | a1  b2  c3  d4  e5  f6  g7  h8 |      | a1 |b8  c7  d6  e5  f4  g3  h2 |
    +--------------------------------+      +--------------------------------+
                    a8                                      h8
                  a7  b8                                  g8  h7
                a6  b7  c8                              f8  g7  h6
              a5  b6  c7  d8                          e8  f7  g6  h5
            a4  b5  c6  d7  e8                      d8  e7  f6  g5  h4
          a3  b4  c5  d6  e7  f8                  c8  d7  e6  f5  g4  h3
        a2  b3  c4  d5  e6  f7  g8              b8  c7  d6  e5  f4  g3  h2
      a1  b2  c3  d4  e5  f6  g7  h8          a8  b7  c6  d5  e4  f3  g2  h1
        b1  c2  d3  e4  f5  g6  h7              a7  b6  c5  d4  e3  f2  g1
          c1  d2  e3  f4  g5  h6                  a6  b5  c4  d3  e2  f1
            d1  e2  f3  g4  h5                      a5  b4  c3  d2  e1
              e1  f2  g3  h4                          a4  b3  c2  d1
                f1  g2  h3                              a3  b2  c1
                  g1  h2                                  a2  b1
                    h1                                      a1

             +--Patterns--+------------+-----------+-----------+
             |  binary p1 |  binary p2 |   hex p1  |   hex p2  |
    +--------+------------+------------+-----------+-----------+
    |        |     01110  |     00001  |    0xE    |    0x1    |
    |        |     01110  |     10000  |    0xE    |    0x10   |
    |  close |    010110  |    100000  |    0x16   |    0x20   |
    | threes |    010110  |    000001  |    0x16   |    0x1    |
    |        |    011010  |    100000  |    0x1A   |    0x20   |
    |        |    011010  |    000001  |    0x1A   |    0x1    |
    +--------+------------+------------+-----------+-----------+
    |        |    011110  |    000001  |    0x1E   |    0x1    |
    |        |    011110  |    100000  |    0x1E   |    0x20   |
    |        |   0101110  |   1000000  |    0x2E   |    0x40   |
    |        |   0101110  |   0000001  |    0x2E   |    0x1    |
    |  close |   0111010  |   1000000  |    0x3A   |    0x40   |
    |  fours |   0111010  |   0000001  |    0x3A   |    0x1    |
    |        |   0110110  |   1000000  |    0x36   |    0x40   |
    |        |   0110110  |   0000001  |    0x36   |    0x1    |
    |        |  01011010  |  10000000  |    0x5A   |    0x80   | ?
    |        |  01011010  |  00000001  |    0x5A   |    0x1    | ?
    +--------+------------+------------+-----------+-----------+
    |  open  |     01110  |     00000  |    0xE    |    0x0    |
    | threes |    010110  |    000000  |    0x16   |    0x0    |
    |        |    011010  |    000000  |    0x1A   |    0x0    |
    +--------+------------+------------+-----------+-----------+
    |        |    011110  |    000000  |    0x1E   |    0x0    |
    |  open  |   0101110  |   0000000  |    0x2E   |    0x0    |
    | fours  |   0111010  |   0000000  |    0x3A   |    0x0    |
    |        |   0110110  |   0000000  |    0x36   |    0x0    |
    |        |  01011010  |  00000000  |    0x5A   |    0x0    |
    +--------+------------+------------+-----------+-----------+
    Patterns take at most 8 bits, in order to detect open and closed
    patterns we must look at both player boards to see if extremities
    of patterns are blocked.


ROTATION 45 CLOCKWISE OPERATIONS (not optimized) for column 2:

    +--9x9 Board-------------------------+ : board
    | a9  b9  c9  d9  e9  f9  g9  h9  i9 |
    | a8  b8  c8  d8  e8  f8  g8  h8  i8 |
    | a7  b7  c7  d7  e7  f7  g7  h7  i7 |
    | a6  b6  c6  d6  e6  f6  g6  h6  i6 |
    | a5  b5  c5  d5  e5  f5  g5  h5  i5 |
    | a4  b4  c4  d4  e4  f4  g4  h4  i4 |
    | a3  b3  c3  d3  e3  f3  g3  h3  i3 |
    | a2  b2  c2  d2  e2  f2  g2  h2  i2 |
    | a1  b1  c1  d1  e1  f1  g1  h1  i1 |
    +------------------------------------+

    +--9x9 Board-------------------------+ : board >> (9 * 2)
    |  .   .   .   .   .   .   .   .   . |
    |  .   .   .   .   .   .   .   .   . |
    | a9  b9  c9  d9  e9  f9  g9  h9  i9 |
    | a8  b8  c8  d8  e8  f8  g8  h8  i8 |
    | a7  b7  c7  d7  e7  f7  g7  h7  i7 |
    | a6  b6  c6  d6  e6  f6  g6  h6  i6 |
    | a5  b5  c5  d5  e5  f5  g5  h5  i5 |
    | a4  b4  c4  d4  e4  f4  g4  h4  i4 |
    | a3  b3  c3  d3  e3  f3  g3  h3  i3 |
    +------------------------------------+

    +--9x9 Board-------------------------+ : board << (9 * (9-2))
    | a2  b2  c2  d2  e2  f2  g2  h2  i2 |
    | a1  b1  c1  d1  e1  f1  g1  h1  i1 |
    |  .   .   .   .   .   .   .   .   . |
    |  .   .   .   .   .   .   .   .   . |
    |  .   .   .   .   .   .   .   .   . |
    |  .   .   .   .   .   .   .   .   . |
    |  .   .   .   .   .   .   .   .   . |
    |  .   .   .   .   .   .   .   .   . |
    +------------------------------------+

    +--9x9 Board-------------------------+ : (board >> (9 * 2)) | (board << (9 * (9-2)))
    | a2  b2  c2  d2  e2  f2  g2  h2  i2 |
    | a1  b1  c1  d1  e1  f1  g1  h1  i1 |
    | a9  b9  c9  d9  e9  f9  g9  h9  i9 |
    | a8  b8  c8  d8  e8  f8  g8  h8  i8 |
    | a7  b7  c7  d7  e7  f7  g7  h7  i7 |
    | a6  b6  c6  d6  e6  f6  g6  h6  i6 |
    | a5  b5  c5  d5  e5  f5  g5  h5  i5 |
    | a4  b4  c4  d4  e4  f4  g4  h4  i4 |
    | a3  b3  c3  d3  e3  f3  g3  h3  i3 |
    +------------------------------------+

    +--9x9 Board-------------------------+ : ( (board >> (9 * 2)) | (board << (9 * (9-2))) ) & mask
    |  .   .  c2   .   .   .   .   .   . |
    |  .   .  c1   .   .   .   .   .   . |
    |  .   .  c9   .   .   .   .   .   . |
    |  .   .  c8   .   .   .   .   .   . |
    |  .   .  c7   .   .   .   .   .   . |
    |  .   .  c6   .   .   .   .   .   . |
    |  .   .  c5   .   .   .   .   .   . |
    |  .   .  c4   .   .   .   .   .   . |
    |  .   .  c3   .   .   .   .   .   . |
    +------------------------------------+


    We want to find the rules to get from the normal 9x9 board
    to the 45deg rotated clockwise Board.

    +--Rotated 9x9 Board 45 Clockwise----+
    | a9 |b1  c2  d3  e4  f5  g6  h7  i8 |
    | a8  b9 |c1  d2  e3  f4  g5  h6  i7 |
    | a7  b8  c9 |d1  e2  f3  g4  h5  i6 |
    | a6  b7  c8  d9 |e1  f2  g3  h4  i5 |
    | a5  b6  c7  d8  e9 |f1  g2  h3  i4 |
    | a4  b5  c6  d7  e8  f9 |g1  h2  i3 |
    | a6  b4  c5  d6  e7  f8  g9 |h1  i2 |
    | a2  b6  c4  d5  e6  f7  g8  h9 |i1 |
    | a1  b2  c6  d4  e5  f6  g7  h8  i9 |
    +------------------------------------+
    | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 /

    We are going to scan each column with a binary mask and the
    intersection operator.


    +-16bit position storage---------+
    |     x    |    y    |    pad    |
    +----------+---------+-----------+-------------+
    | 1 1 1 1 1|1 1 1 1 1|0 0 0 0 0 0| binary mask |
    +----------+---------+-----------+-------------+
    |  0xF800  |  0x7C0  |           |    hex mask |
    +----------+---------+-----------+-------------+

    Encode :
     $ uint16_t pos = (x << 11) | (y << 6);
    Decode :
     $ x = pos & 0xF800;
     $ y = pos & 0x7C0;

     1001001111000000 is (18, 15)

    get_all_neighbours(p1, p2) :
     p1                p2                (p1 | p2)         di(p1 | p2)      di(p1 | p2) ^ (p1 | p2)
     +-----------+     +-----------+     +-----------+     +-----------+    +-----------+
     | . . . . . |     | . . . . . |     | . . . . . |     | 1 1 1 . . |    | 1 1 1 . . |
     | . 1 1 . . |     | 1 . . . . |     | 1 1 1 . . |     | 1 1 1 1 . |    | . . . 1 . |
     | . . . . . |     | . . . . . |     | . . . . . |     | 1 1 1 1 . |    | 1 1 . 1 . |
     | . . . 1 . |     | . . 1 . . |     | . . 1 1 . |     | . 1 1 1 1 |    | . 1 . . 1 |
     | . . . . . |     | . . . 1 . |     | . . . 1 . |     | . . 1 1 1 |    | . . 1 . 1 |
     +-----------+     +-----------+     +-----------+     +-----------+    +-----------+


     +-----------+     +-----------+     +-----------+     +-----------+    +-----------+
     | . . . . . |     | . . . . 1 |     | . . . . . |     | . . . . . |    | . . . . . |
     | . . . . . |     | 1 1 . . 1 |     | 1 1 . . . |     | . . . . . |    | . . . . . |
     | 1 . . . . |     | 1 1 . . 1 |     | 1 1 . . . |     | . . . . . |    | . . . . . |
     | . . . . . |     | 1 1 1 1 1 |     | 1 1 1 1 1 |     | . . . . . |    | . . . . . |
     | . . . 1 . |     | . . 1 1 1 |     | . . 1 1 1 |     | . . . . . |    | . . . . . |
     +-----------+     +-----------+     +-----------+     +-----------+    +-----------+

*/