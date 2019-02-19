/*
    Cinnamon UCI chess engine
    Copyright (C) Giuseppe Cannella

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

#include "def.h"
#include <string.h>
#include <sstream>
#include <array>

using namespace _def;
namespace _board {

    static const string NAME = "Cinnamon 2.1-SNAPSHOT";
    static const string STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    static constexpr int MAX_PLY = 96;

    static constexpr bool USE_HASH_YES = true;
    static constexpr bool USE_HASH_NO = false;

    static constexpr bool SMP_YES = true;
    static constexpr bool SMP_NO = false;

    static constexpr int _INFINITE = 32000;

    const string EPD2PGN_HELP = "-epd2pgn -f epd_file [-m max_pieces]";
    const string PERFT_HELP = "-perft [-d depth] [-c nCpu] [-h hash size (mb) [-F dump file]] [-f \"fen position\"]";
    const string DTM_HELP = "-dtm -f \"fen position\" [-p path] [-s scheme] [-i installed pieces]";
    const string PUZZLE_HELP = "-puzzle_epd -t KxyKnm ex: KRKP | KQKP | KBBKN | KQKR | KRKB | KRKN";

    static constexpr int PAWN_BLACK = 0;
    static constexpr int PAWN_WHITE = 1;
    static constexpr int ROOK_BLACK = 2;
    static constexpr int ROOK_WHITE = 3;
    static constexpr int BISHOP_BLACK = 4;
    static constexpr int BISHOP_WHITE = 5;
    static constexpr int KNIGHT_BLACK = 6;
    static constexpr int KNIGHT_WHITE = 7;
    static constexpr int KING_BLACK = 8;
    static constexpr int KING_WHITE = 9;
    static constexpr int QUEEN_BLACK = 10;
    static constexpr int QUEEN_WHITE = 11;
    static constexpr int SQUARE_FREE = 12;


    static constexpr int VALUEPAWN = 100;
    static constexpr int VALUEROOK = 520;
    static constexpr int VALUEBISHOP = 335;
    static constexpr int VALUEKNIGHT = 330;
    static constexpr int VALUEQUEEN = 980;
    static constexpr int VALUEKING = _INFINITE;

    static constexpr array<int, 13> PIECES_VALUE = {VALUEPAWN, VALUEPAWN, VALUEROOK, VALUEROOK, VALUEBISHOP,
                                                    VALUEBISHOP, VALUEKNIGHT, VALUEKNIGHT, VALUEKING, VALUEKING,
                                                    VALUEQUEEN, VALUEQUEEN, 0};

    static constexpr u64 CENTER_MASK = 0x1818000000ULL;
    static constexpr u64 BIG_DIAGONAL = 0x102040810204080ULL;
    static constexpr u64 BIG_ANTIDIAGONAL = 0x8040201008040201ULL;


    typedef struct {
        char promotionPiece;
        char pieceFrom;
        uchar capturedPiece;
        uchar from;
        uchar to;
        char side;
        uchar type;
        int score;
        bool used;
    } _Tmove;

    typedef struct {
        _Tmove *moveList;
        int size;
    } _TmoveP;

    typedef struct {
        int cmove;
        _Tmove argmove[MAX_PLY];
    } _TpvLine;

    static constexpr u64 POW2_0 = 0x1ULL;
    static constexpr u64 POW2_1 = 0x2ULL;
    static constexpr u64 POW2_2 = 0x4ULL;
    static constexpr u64 POW2_3 = 0x8ULL;
    static constexpr u64 POW2_4 = 0x10ULL;
    static constexpr u64 POW2_5 = 0x20ULL;
    static constexpr u64 POW2_6 = 0x40ULL;
    static constexpr u64 POW2_7 = 0x80ULL;
    static constexpr u64 POW2_56 = 0x100000000000000ULL;
    static constexpr u64 POW2_57 = 0x200000000000000ULL;
    static constexpr u64 POW2_58 = 0x400000000000000ULL;
    static constexpr u64 POW2_59 = 0x800000000000000ULL;
    static constexpr u64 POW2_60 = 0x1000000000000000ULL;
    static constexpr u64 POW2_61 = 0x2000000000000000ULL;
    static constexpr u64 POW2_62 = 0x4000000000000000ULL;
    static constexpr u64 POW2_63 = 0x8000000000000000ULL;
    static constexpr u64 NOTPOW2_0 = 0xfffffffffffffffeULL;
    static constexpr u64 NOTPOW2_1 = 0xfffffffffffffffdULL;
    static constexpr u64 NOTPOW2_2 = 0xfffffffffffffffbULL;
    static constexpr u64 NOTPOW2_3 = 0xfffffffffffffff7ULL;
    static constexpr u64 NOTPOW2_4 = 0xffffffffffffffefULL;
    static constexpr u64 NOTPOW2_5 = 0xffffffffffffffdfULL;
    static constexpr u64 NOTPOW2_7 = 0xffffffffffffff7fULL;
    static constexpr u64 NOTPOW2_56 = 0xfeffffffffffffffULL;
    static constexpr u64 NOTPOW2_57 = 0xfdffffffffffffffULL;
    static constexpr u64 NOTPOW2_58 = 0xfbffffffffffffffULL;
    static constexpr u64 NOTPOW2_59 = 0xf7ffffffffffffffULL;
    static constexpr u64 NOTPOW2_60 = 0xefffffffffffffffULL;
    static constexpr u64 NOTPOW2_61 = 0xdfffffffffffffffULL;
    static constexpr u64 NOTPOW2_63 = 0x7fffffffffffffffULL;


    static const array<string, 64> BOARD = {"h1", "g1", "f1", "e1", "d1", "c1", "b1", "a1", "h2", "g2", "f2", "e2",
                                            "d2", "c2", "b2", "a2", "h3", "g3", "f3", "e3", "d3", "c3", "b3", "a3",
                                            "h4", "g4", "f4", "e4", "d4", "c4", "b4", "a4", "h5", "g5", "f5", "e5",
                                            "d5", "c5", "b5", "a5", "h6", "g6", "f6", "e6", "d6", "c6", "b6", "a6",
                                            "h7", "g7", "f7", "e7", "d7", "c7", "b7", "a7", "h8", "g8", "f8", "e8",
                                            "d8", "c8", "b8", "a8"};
    static constexpr array<char, 13> FEN_PIECE = {'p', 'P', 'r', 'R', 'b', 'B', 'n', 'N', 'k', 'K', 'q', 'Q', '-'};

    static constexpr array<u64, 2> BISHOP_HOME = {0x24ULL, 0x2400000000000000ULL};
    static constexpr array<u64, 2> KNIGHT_HOME = {0x4200000000000000ULL, 0x42ULL};
    static constexpr array<u64, 2> RANK_1_7 = {0xff00ULL, 0xff000000000000ULL};

    static constexpr array<int, 255> INV_FEN = {
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x00000005, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x00000009, 0x000000ff, 0x000000ff, 0x00000007, 0x000000ff,
            0x00000001, 0x0000000b, 0x00000003, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x00000004, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x00000008, 0x000000ff, 0x000000ff, 0x00000006, 0x000000ff,
            0x00000000, 0x0000000a, 0x00000002, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff,
            0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff, 0x000000ff
    };

    static constexpr array<u64, 64> RANK_BOUND = {
            0x0000000000000002ULL, 0x0000000000000005ULL, 0x000000000000000aULL, 0x0000000000000014ULL,
            0x0000000000000028ULL, 0x0000000000000050ULL, 0x00000000000000a0ULL, 0x0000000000000040ULL,
            0x0000000000000200ULL, 0x0000000000000500ULL, 0x0000000000000a00ULL, 0x0000000000001400ULL,
            0x0000000000002800ULL, 0x0000000000005000ULL, 0x000000000000a000ULL, 0x0000000000004000ULL,
            0x0000000000020000ULL, 0x0000000000050000ULL, 0x00000000000a0000ULL, 0x0000000000140000ULL,
            0x0000000000280000ULL, 0x0000000000500000ULL, 0x0000000000a00000ULL, 0x0000000000400000ULL,
            0x0000000002000000ULL, 0x0000000005000000ULL, 0x000000000a000000ULL, 0x0000000014000000ULL,
            0x0000000028000000ULL, 0x0000000050000000ULL, 0x00000000a0000000ULL, 0x0000000040000000ULL,
            0x0000000200000000ULL, 0x0000000500000000ULL, 0x0000000a00000000ULL, 0x0000001400000000ULL,
            0x0000002800000000ULL, 0x0000005000000000ULL, 0x000000a000000000ULL, 0x0000004000000000ULL,
            0x0000020000000000ULL, 0x0000050000000000ULL, 0x00000a0000000000ULL, 0x0000140000000000ULL,
            0x0000280000000000ULL, 0x0000500000000000ULL, 0x0000a00000000000ULL, 0x0000400000000000ULL,
            0x0002000000000000ULL, 0x0005000000000000ULL, 0x000a000000000000ULL, 0x0014000000000000ULL,
            0x0028000000000000ULL, 0x0050000000000000ULL, 0x00a0000000000000ULL, 0x0040000000000000ULL,
            0x0200000000000000ULL, 0x0500000000000000ULL, 0x0a00000000000000ULL, 0x1400000000000000ULL,
            0x2800000000000000ULL, 0x5000000000000000ULL, 0xa000000000000000ULL, 0x4000000000000000ULL
    };

    static constexpr array<u64, 64> RANK = {
            0x00000000000000ffULL, 0x00000000000000ffULL, 0x00000000000000ffULL, 0x00000000000000ffULL,
            0x00000000000000ffULL, 0x00000000000000ffULL, 0x00000000000000ffULL, 0x00000000000000ffULL,
            0x000000000000ff00ULL, 0x000000000000ff00ULL, 0x000000000000ff00ULL, 0x000000000000ff00ULL,
            0x000000000000ff00ULL, 0x000000000000ff00ULL, 0x000000000000ff00ULL, 0x000000000000ff00ULL,
            0x0000000000ff0000ULL, 0x0000000000ff0000ULL, 0x0000000000ff0000ULL, 0x0000000000ff0000ULL,
            0x0000000000ff0000ULL, 0x0000000000ff0000ULL, 0x0000000000ff0000ULL, 0x0000000000ff0000ULL,
            0x00000000ff000000ULL, 0x00000000ff000000ULL, 0x00000000ff000000ULL, 0x00000000ff000000ULL,
            0x00000000ff000000ULL, 0x00000000ff000000ULL, 0x00000000ff000000ULL, 0x00000000ff000000ULL,
            0x000000ff00000000ULL, 0x000000ff00000000ULL, 0x000000ff00000000ULL, 0x000000ff00000000ULL,
            0x000000ff00000000ULL, 0x000000ff00000000ULL, 0x000000ff00000000ULL, 0x000000ff00000000ULL,
            0x0000ff0000000000ULL, 0x0000ff0000000000ULL, 0x0000ff0000000000ULL, 0x0000ff0000000000ULL,
            0x0000ff0000000000ULL, 0x0000ff0000000000ULL, 0x0000ff0000000000ULL, 0x0000ff0000000000ULL,
            0x00ff000000000000ULL, 0x00ff000000000000ULL, 0x00ff000000000000ULL, 0x00ff000000000000ULL,
            0x00ff000000000000ULL, 0x00ff000000000000ULL, 0x00ff000000000000ULL, 0x00ff000000000000ULL,
            0xff00000000000000ULL, 0xff00000000000000ULL, 0xff00000000000000ULL, 0xff00000000000000ULL,
            0xff00000000000000ULL, 0xff00000000000000ULL, 0xff00000000000000ULL, 0xff00000000000000ULL
    };

    static constexpr array<u64, 64> ANTIDIAGONAL = {
            0x8040201008040201ULL, 0x80402010080402ULL, 0x804020100804ULL, 0x8040201008ULL, 0x80402010ULL, 0x804020ULL,
            0x8040ULL, 0x80ULL, 0x4020100804020100ULL, 0x8040201008040201ULL, 0x80402010080402ULL, 0x804020100804ULL,
            0x8040201008ULL, 0x80402010ULL, 0x804020ULL, 0x8040ULL, 0x2010080402010000ULL, 0x4020100804020100ULL,
            0x8040201008040201ULL, 0x80402010080402ULL, 0x804020100804ULL, 0x8040201008ULL, 0x80402010ULL, 0x804020ULL,
            0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
            0x80402010080402ULL, 0x804020100804ULL, 0x8040201008ULL, 0x80402010ULL, 0x804020100000000ULL,
            0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
            0x80402010080402ULL, 0x804020100804ULL, 0x8040201008ULL, 0x402010000000000ULL, 0x804020100000000ULL,
            0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
            0x80402010080402ULL, 0x804020100804ULL, 0x201000000000000ULL, 0x402010000000000ULL, 0x804020100000000ULL,
            0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
            0x80402010080402ULL, 0x100000000000000ULL, 0x201000000000000ULL, 0x402010000000000ULL, 0x804020100000000ULL,
            0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL};

    static constexpr array<u64, 64> DIAGONAL = {
            0x1ULL, 0x102ULL, 0x10204ULL, 0x1020408ULL, 0x102040810ULL, 0x10204081020ULL, 0x1020408102040ULL,
            0x102040810204080ULL, 0x102ULL, 0x10204ULL, 0x1020408ULL, 0x102040810ULL, 0x10204081020ULL,
            0x1020408102040ULL, 0x102040810204080ULL, 0x204081020408000ULL, 0x10204ULL, 0x1020408ULL, 0x102040810ULL,
            0x10204081020ULL, 0x1020408102040ULL, 0x102040810204080ULL, 0x204081020408000ULL, 0x408102040800000ULL,
            0x1020408ULL, 0x102040810ULL, 0x10204081020ULL, 0x1020408102040ULL, 0x102040810204080ULL,
            0x204081020408000ULL, 0x408102040800000ULL, 0x810204080000000ULL, 0x102040810ULL, 0x10204081020ULL,
            0x1020408102040ULL, 0x102040810204080ULL, 0x204081020408000ULL, 0x408102040800000ULL, 0x810204080000000ULL,
            0x1020408000000000ULL, 0x10204081020ULL, 0x1020408102040ULL, 0x102040810204080ULL, 0x204081020408000ULL,
            0x408102040800000ULL, 0x810204080000000ULL, 0x1020408000000000ULL, 0x2040800000000000ULL,
            0x1020408102040ULL, 0x102040810204080ULL, 0x204081020408000ULL, 0x408102040800000ULL,
            0x810204080000000ULL, 0x1020408000000000ULL, 0x2040800000000000ULL, 0x4080000000000000ULL,
            0x102040810204080ULL, 0x204081020408000ULL, 0x408102040800000ULL, 0x810204080000000ULL,
            0x1020408000000000ULL, 0x2040800000000000ULL, 0x4080000000000000ULL, 0x8000000000000000ULL
    };

    static constexpr array<u64, 64> DIAGONAL_ANTIDIAGONAL = {
            0x8040201008040200ULL, 0x0080402010080500ULL, 0x0000804020110a00ULL, 0x0000008041221400ULL,
            0x0000000182442800ULL, 0x0000010204885000ULL, 0x000102040810a000ULL, 0x0102040810204000ULL,
            0x4020100804020002ULL, 0x8040201008050005ULL, 0x00804020110a000aULL, 0x0000804122140014ULL,
            0x0000018244280028ULL, 0x0001020488500050ULL, 0x0102040810a000a0ULL, 0x0204081020400040ULL,
            0x2010080402000204ULL, 0x4020100805000508ULL, 0x804020110a000a11ULL, 0x0080412214001422ULL,
            0x0001824428002844ULL, 0x0102048850005088ULL, 0x02040810a000a010ULL, 0x0408102040004020ULL,
            0x1008040200020408ULL, 0x2010080500050810ULL, 0x4020110a000a1120ULL, 0x8041221400142241ULL,
            0x0182442800284482ULL, 0x0204885000508804ULL, 0x040810a000a01008ULL, 0x0810204000402010ULL,
            0x0804020002040810ULL, 0x1008050005081020ULL, 0x20110a000a112040ULL, 0x4122140014224180ULL,
            0x8244280028448201ULL, 0x0488500050880402ULL, 0x0810a000a0100804ULL, 0x1020400040201008ULL,
            0x0402000204081020ULL, 0x0805000508102040ULL, 0x110a000a11204080ULL, 0x2214001422418000ULL,
            0x4428002844820100ULL, 0x8850005088040201ULL, 0x10a000a010080402ULL, 0x2040004020100804ULL,
            0x0200020408102040ULL, 0x0500050810204080ULL, 0x0a000a1120408000ULL, 0x1400142241800000ULL,
            0x2800284482010000ULL, 0x5000508804020100ULL, 0xa000a01008040201ULL, 0x4000402010080402ULL,
            0x0002040810204080ULL, 0x0005081020408000ULL, 0x000a112040800000ULL, 0x0014224180000000ULL,
            0x0028448201000000ULL, 0x0050880402010000ULL, 0x00a0100804020100ULL, 0x0040201008040201ULL
    };

    static constexpr array<char, 64> FILE_AT = {0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                                0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,
                                                0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7};

    static constexpr array<char, 64> RANK_ATx8 = {0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL,
                                                  0x8ULL, 0x8ULL, 0x8ULL, 0x8ULL, 0x8ULL, 0x8ULL, 0x8ULL, 0x8ULL,
                                                  0x10ULL, 0x10ULL, 0x10ULL, 0x10ULL, 0x10ULL, 0x10ULL, 0x10ULL,
                                                  0x10ULL, 0x18ULL, 0x18ULL, 0x18ULL, 0x18ULL, 0x18ULL, 0x18ULL,
                                                  0x18ULL, 0x18ULL, 0x20ULL, 0x20ULL, 0x20ULL, 0x20ULL, 0x20ULL,
                                                  0x20ULL, 0x20ULL, 0x20ULL, 0x28ULL, 0x28ULL, 0x28ULL, 0x28ULL,
                                                  0x28ULL, 0x28ULL, 0x28ULL, 0x28ULL, 0x30ULL, 0x30ULL, 0x30ULL,
                                                  0x30ULL, 0x30ULL, 0x30ULL, 0x30ULL, 0x30ULL, 0x38ULL, 0x38ULL,
                                                  0x38ULL, 0x38ULL, 0x38ULL, 0x38ULL, 0x38ULL, 0x38ULL};

    static constexpr array<char, 64> RANK_AT = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
                                                3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,
                                                6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7};

    static constexpr array<u64, 2> PAWNS_JUMP = {0xFF000000000000ULL, 0xFF00ULL};
    static constexpr array<u64, 2> RANK_1_8 = {0xff00000000000000ULL, 0xffULL};
    static constexpr array<u64, 2> NO_FILE_LEFT = {0xFEFEFEFEFEFEFEFEULL, 0x7F7F7F7F7F7F7F7FULL};
    static constexpr array<u64, 2> NO_FILE_RIGHT = {0x7F7F7F7F7F7F7F7FULL, 0xFEFEFEFEFEFEFEFEULL};

    static constexpr array<u64, 2> PAWNS_7_2 = {0xFF00ULL, 0xFF000000000000ULL};

    static constexpr array<u64, 64> FILE_ = {
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL,
            0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL, 0x0808080808080808ULL,
            0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL
    };
    static constexpr u64 POW2[64] = {0x1ULL, 0x2ULL, 0x4ULL, 0x8ULL, 0x10ULL, 0x20ULL, 0x40ULL, 0x80ULL, 0x100ULL,
                                     0x200ULL, 0x400ULL, 0x800ULL, 0x1000ULL, 0x2000ULL, 0x4000ULL, 0x8000ULL,
                                     0x10000ULL, 0x20000ULL, 0x40000ULL, 0x80000ULL, 0x100000ULL, 0x200000ULL,
                                     0x400000ULL, 0x800000ULL, 0x1000000ULL, 0x2000000ULL, 0x4000000ULL, 0x8000000ULL,
                                     0x10000000ULL, 0x20000000ULL, 0x40000000ULL, 0x80000000ULL, 0x100000000ULL,
                                     0x200000000ULL, 0x400000000ULL, 0x800000000ULL, 0x1000000000ULL, 0x2000000000ULL,
                                     0x4000000000ULL, 0x8000000000ULL, 0x10000000000ULL, 0x20000000000ULL,
                                     0x40000000000ULL, 0x80000000000ULL, 0x100000000000ULL, 0x200000000000ULL,
                                     0x400000000000ULL, 0x800000000000ULL, 0x1000000000000ULL, 0x2000000000000ULL,
                                     0x4000000000000ULL, 0x8000000000000ULL, 0x10000000000000ULL, 0x20000000000000ULL,
                                     0x40000000000000ULL, 0x80000000000000ULL, 0x100000000000000ULL,
                                     0x200000000000000ULL, 0x400000000000000ULL, 0x800000000000000ULL,
                                     0x1000000000000000ULL, 0x2000000000000000ULL, 0x4000000000000000ULL,
                                     0x8000000000000000ULL};

    static constexpr array<u64, 64> NOTPOW2 = {0xfffffffffffffffeULL, 0xfffffffffffffffdULL, 0xfffffffffffffffbULL,
                                               0xfffffffffffffff7ULL, 0xffffffffffffffefULL, 0xffffffffffffffdfULL,
                                               0xffffffffffffffbfULL, 0xffffffffffffff7fULL, 0xfffffffffffffeffULL,
                                               0xfffffffffffffdffULL, 0xfffffffffffffbffULL, 0xfffffffffffff7ffULL,
                                               0xffffffffffffefffULL, 0xffffffffffffdfffULL, 0xffffffffffffbfffULL,
                                               0xffffffffffff7fffULL, 0xfffffffffffeffffULL, 0xfffffffffffdffffULL,
                                               0xfffffffffffbffffULL, 0xfffffffffff7ffffULL, 0xffffffffffefffffULL,
                                               0xffffffffffdfffffULL, 0xffffffffffbfffffULL, 0xffffffffff7fffffULL,
                                               0xfffffffffeffffffULL, 0xfffffffffdffffffULL, 0xfffffffffbffffffULL,
                                               0xfffffffff7ffffffULL, 0xffffffffefffffffULL, 0xffffffffdfffffffULL,
                                               0xffffffffbfffffffULL, 0xffffffff7fffffffULL, 0xfffffffeffffffffULL,
                                               0xfffffffdffffffffULL, 0xfffffffbffffffffULL, 0xfffffff7ffffffffULL,
                                               0xffffffefffffffffULL, 0xffffffdfffffffffULL, 0xffffffbfffffffffULL,
                                               0xffffff7fffffffffULL, 0xfffffeffffffffffULL, 0xfffffdffffffffffULL,
                                               0xfffffbffffffffffULL, 0xfffff7ffffffffffULL, 0xffffefffffffffffULL,
                                               0xffffdfffffffffffULL, 0xffffbfffffffffffULL,
                                               0xffff7fffffffffffULL, 0xfffeffffffffffffULL, 0xfffdffffffffffffULL,
                                               0xfffbffffffffffffULL, 0xfff7ffffffffffffULL, 0xffefffffffffffffULL,
                                               0xffdfffffffffffffULL, 0xffbfffffffffffffULL, 0xff7fffffffffffffULL,
                                               0xfeffffffffffffffULL, 0xfdffffffffffffffULL, 0xfbffffffffffffffULL,
                                               0xf7ffffffffffffffULL, 0xefffffffffffffffULL, 0xdfffffffffffffffULL,
                                               0xbfffffffffffffffULL, 0x7fffffffffffffffULL};


    static constexpr array<array<u64, 64>, 2> PAWN_FORK_MASK = {{{
                                                                         0, 0, 0, 0, 0, 0, 0, 0,
                                                                         0x2ULL, 0x5ULL, 0xaULL, 0x14ULL, 0x28ULL, 0x50ULL, 0xa0ULL, 0x40ULL,
                                                                         0x200ULL, 0x500ULL, 0xa00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xa000ULL, 0x4000ULL,
                                                                         0x20000ULL, 0x50000ULL, 0xa0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL,
                                                                         0x2000000ULL, 0x5000000ULL, 0xa000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL,
                                                                         0x200000000ULL, 0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xa000000000ULL, 0x4000000000ULL,
                                                                         0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL, 0x280000000000ULL, 0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL,
                                                                         0x0002000000000000ULL, 0x0005000000000000ULL, 0x000A000000000000ULL, 0x0014000000000000ULL, 0x0028000000000000ULL, 0x0050000000000000ULL, 0x00A0000000000000ULL, 0x0040000000000000ULL},
                                                                        {0x0000000000000200ULL, 0x0000000000000500ULL, 0x0000000000000A00ULL, 0x0000000000001400ULL, 0x0000000000002800ULL, 0x0000000000005000ULL, 0x000000000000A000ULL, 0x0000000000004000ULL,
                                                                                0x20000ULL, 0x50000ULL, 0xa0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL,
                                                                                0x2000000ULL, 0x5000000ULL, 0xa000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL,
                                                                                0x200000000ULL, 0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xa000000000ULL, 0x4000000000ULL,
                                                                                0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL, 0x280000000000ULL, 0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL,
                                                                                0x2000000000000ULL, 0x5000000000000ULL, 0xa000000000000ULL, 0x14000000000000ULL, 0x28000000000000ULL, 0x50000000000000ULL, 0xa0000000000000ULL, 0x40000000000000ULL,
                                                                                0x200000000000000ULL, 0x500000000000000ULL, 0xa00000000000000ULL, 0x1400000000000000ULL, 0x2800000000000000ULL, 0x5000000000000000ULL, 0xa000000000000000ULL, 0x4000000000000000ULL,
                                                                                0, 0, 0, 0, 0, 0, 0, 0}}};

    static constexpr array<u64, 64> NEAR_MASK1 = {0x0000000000000302ULL, 0x0000000000000705ULL, 0x0000000000000E0AULL,
                                                  0x0000000000001C14ULL, 0x0000000000003828ULL, 0x0000000000007050ULL,
                                                  0x000000000000E0A0ULL, 0x000000000000C040ULL, 0x0000000000030203ULL,
                                                  0x0000000000070507ULL, 0x00000000000E0A0EULL, 0x00000000001C141CULL,
                                                  0x0000000000382838ULL, 0x0000000000705070ULL, 0x0000000000E0A0E0ULL,
                                                  0x0000000000C040C0ULL, 0x0000000003020300ULL, 0x0000000007050700ULL,
                                                  0x000000000E0A0E00ULL, 0x000000001C141C00ULL, 0x0000000038283800ULL,
                                                  0x0000000070507000ULL, 0x00000000E0A0E000ULL, 0x00000000C040C000ULL,
                                                  0x0000000302030000ULL, 0x0000000705070000ULL, 0x0000000E0A0E0000ULL,
                                                  0x0000001C141C0000ULL, 0x0000003828380000ULL, 0x0000007050700000ULL,
                                                  0x000000E0A0E00000ULL, 0x000000C040C00000ULL, 0x0000030203000000ULL,
                                                  0x0000070507000000ULL, 0x00000E0A0E000000ULL, 0x00001C141C000000ULL,
                                                  0x0000382838000000ULL, 0x0000705070000000ULL, 0x0000E0A0E0000000ULL,
                                                  0x0000C040C0000000ULL, 0x0003020300000000ULL, 0x0007050700000000ULL,
                                                  0x000E0A0E00000000ULL, 0x001C141C00000000ULL, 0x0038283800000000ULL,
                                                  0x0070507000000000ULL, 0x00E0A0E000000000ULL,
                                                  0x00C040C000000000ULL, 0x0302030000000000ULL, 0x0705070000000000ULL,
                                                  0x0E0A0E0000000000ULL, 0x1C141C0000000000ULL, 0x3828380000000000ULL,
                                                  0x7050700000000000ULL, 0xE0A0E00000000000ULL, 0xC040C00000000000ULL,
                                                  0x0203000000000000ULL, 0x0507000000000000ULL, 0x0A0E000000000000ULL,
                                                  0x141C000000000000ULL, 0x2838000000000000ULL, 0x5070000000000000ULL,
                                                  0xA0E0000000000000ULL, 0x40C0000000000000ULL};

    static constexpr array<u64, 64> NEAR_MASK2 = {0x70706ULL, 0xf0f0dULL, 0x1f1f1bULL, 0x3e3e36ULL, 0x7c7c6cULL,
                                                  0xf8f8d8ULL, 0xf0f0b0ULL, 0xe0e060ULL, 0x7070607ULL, 0xf0f0d0fULL,
                                                  0x1f1f1b1fULL, 0x3e3e363eULL, 0x7c7c6c7cULL, 0xf8f8d8f8ULL,
                                                  0xf0f0b0f0ULL, 0xe0e060e0ULL, 0x707060707ULL, 0xf0f0d0f0fULL,
                                                  0x1f1f1b1f1fULL, 0x3e3e363e3eULL, 0x7c7c6c7c7cULL, 0xf8f8d8f8f8ULL,
                                                  0xf0f0b0f0f0ULL, 0xe0e060e0e0ULL, 0x70706070700ULL, 0xf0f0d0f0f00ULL,
                                                  0x1f1f1b1f1f00ULL, 0x3e3e363e3e00ULL, 0x7c7c6c7c7c00ULL,
                                                  0xf8f8d8f8f800ULL, 0xf0f0b0f0f000ULL, 0xe0e060e0e000ULL,
                                                  0x7070607070000ULL, 0xf0f0d0f0f0000ULL, 0x1f1f1b1f1f0000ULL,
                                                  0x3e3e363e3e0000ULL, 0x7c7c6c7c7c0000ULL, 0xf8f8d8f8f80000ULL,
                                                  0xf0f0b0f0f00000ULL, 0xe0e060e0e00000ULL, 0x707060707000000ULL,
                                                  0xf0f0d0f0f000000ULL, 0x1f1f1b1f1f000000ULL, 0x3e3e363e3e000000ULL,
                                                  0x7c7c6c7c7c000000ULL, 0xf8f8d8f8f8000000ULL, 0xf0f0b0f0f0000000ULL,
                                                  0xe0e060e0e0000000ULL, 0x706070700000000ULL, 0xf0d0f0f00000000ULL,
                                                  0x1f1b1f1f00000000ULL, 0x3e363e3e00000000ULL, 0x7c6c7c7c00000000ULL,
                                                  0xf8d8f8f800000000ULL, 0xf0b0f0f000000000ULL, 0xe060e0e000000000ULL,
                                                  0x607070000000000ULL, 0xd0f0f0000000000ULL,
                                                  0x1b1f1f0000000000ULL, 0x363e3e0000000000ULL, 0x6c7c7c0000000000ULL,
                                                  0xd8f8f80000000000ULL, 0xb0f0f00000000000ULL, 0x60e0e00000000000ULL};

    static constexpr array<u64, 64> KNIGHT_MASK = {0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000A1100ULL,
                                                   0x0000000000142200ULL, 0x0000000000284400ULL, 0x0000000000508800ULL,
                                                   0x0000000000A01000ULL, 0x0000000000402000ULL, 0x0000000002040004ULL,
                                                   0x0000000005080008ULL, 0x000000000A110011ULL, 0x0000000014220022ULL,
                                                   0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000A0100010ULL,
                                                   0x0000000040200020ULL, 0x0000000204000402ULL, 0x0000000508000805ULL,
                                                   0x0000000A1100110AULL, 0x0000001422002214ULL, 0x0000002844004428ULL,
                                                   0x0000005088008850ULL, 0x000000A0100010A0ULL, 0x0000004020002040ULL,
                                                   0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000A1100110A00ULL,
                                                   0x0000142200221400ULL, 0x0000284400442800ULL, 0x0000508800885000ULL,
                                                   0x0000A0100010A000ULL, 0x0000402000204000ULL, 0x0002040004020000ULL,
                                                   0x0005080008050000ULL, 0x000A1100110A0000ULL, 0x0014220022140000ULL,
                                                   0x0028440044280000ULL, 0x0050880088500000ULL, 0x00A0100010A00000ULL,
                                                   0x0040200020400000ULL, 0x0204000402000000ULL, 0x0508000805000000ULL,
                                                   0x0A1100110A000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL,
                                                   0x5088008850000000ULL, 0xA0100010A0000000ULL,
                                                   0x4020002040000000ULL, 0x0400040200000000ULL, 0x0800080500000000ULL,
                                                   0x1100110A00000000ULL, 0x2200221400000000ULL, 0x4400442800000000ULL,
                                                   0x8800885000000000ULL, 0x100010A000000000ULL, 0x2000204000000000ULL,
                                                   0x0004020000000000ULL, 0x0008050000000000ULL, 0x00110A0000000000ULL,
                                                   0x0022140000000000ULL, 0x0044280000000000ULL, 0x0088500000000000ULL,
                                                   0x0010A00000000000ULL, 0x0020400000000000ULL};

    static constexpr array<array<u64, 64>, 2> ENPASSANT_MASK = {{{0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0x200000000ULL, 0x500000000ULL, 0xA00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xA000000000ULL, 0x4000000000ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0},
                                                                        {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0x2000000ULL, 0x5000000ULL, 0xA000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xA0000000ULL, 0x40000000ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0}}};


    static constexpr uchar DISTANCE[64][64] = {
            {0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 4, 4, 4, 4, 4, 5, 6, 7, 5, 5, 5, 5, 5, 5, 6, 7, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7,},
            {1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 4, 4, 4, 4, 4, 5, 6, 7, 5, 5, 5, 5, 5, 5, 6, 7, 6, 6, 6, 6, 6, 6, 6, 7,},
            {2, 2, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 4, 4, 4, 4, 4, 5, 6, 7, 5, 5, 5, 5, 5, 5, 6, 7,},
            {3, 3, 3, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 4, 4, 4, 4, 4, 5, 6, 7,},
            {4, 4, 4, 4, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7,},
            {5, 5, 5, 5, 5, 5, 6, 7, 4, 4, 4, 4, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7,},
            {6, 6, 6, 6, 6, 6, 6, 7, 5, 5, 5, 5, 5, 5, 6, 7, 4, 4, 4, 4, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7,},
            {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 7, 5, 5, 5, 5, 5, 5, 6, 7, 4, 4, 4, 4, 4, 5, 6, 7, 3, 3, 3, 3, 4, 5, 6, 7, 2, 2, 2, 3, 4, 5, 6, 7, 1, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7,},
            {1, 0, 1, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,},
            {1, 1, 1, 2, 3, 4, 5, 6, 1, 0, 1, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6,},
            {2, 2, 2, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 1, 0, 1, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 4, 4, 4, 4, 4, 4, 5, 6, 5, 5, 5, 5, 5, 5, 5, 6,},
            {3, 3, 3, 3, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 1, 0, 1, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 4, 4, 4, 4, 4, 4, 5, 6,},
            {4, 4, 4, 4, 4, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 1, 0, 1, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6,},
            {5, 5, 5, 5, 5, 5, 5, 6, 4, 4, 4, 4, 4, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 1, 0, 1, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6,},
            {6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 6, 4, 4, 4, 4, 4, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 1, 0, 1, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6,},
            {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 6, 4, 4, 4, 4, 4, 4, 5, 6, 3, 3, 3, 3, 3, 4, 5, 6, 2, 2, 2, 2, 3, 4, 5, 6, 1, 1, 1, 2, 3, 4, 5, 6, 1, 0, 1, 2, 3, 4, 5, 6,},
            {2, 1, 0, 1, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,},
            {2, 1, 1, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,},
            {2, 2, 2, 2, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5,},
            {3, 3, 3, 3, 3, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5,},
            {4, 4, 4, 4, 4, 4, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5,},
            {5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5,},
            {6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5,},
            {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 5, 3, 3, 3, 3, 3, 3, 4, 5, 2, 2, 2, 2, 2, 3, 4, 5, 2, 1, 1, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 5,},
            {3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,},
            {3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,},
            {3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,},
            {3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,},
            {4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4,},
            {5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4,},
            {6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4,},
            {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4,},
            {4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,},
            {4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,},
            {4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,},
            {4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,},
            {4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 3, 3, 3, 3, 3, 3,},
            {5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 2, 2, 2, 2, 3,},
            {6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3,},
            {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 2, 2, 2, 3, 4, 3, 2, 1, 1, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3,},
            {5, 4, 3, 2, 1, 0, 1, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,},
            {5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 1, 0, 1, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,},
            {5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 1, 0, 1, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,},
            {5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 1, 0, 1, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 4, 4, 4, 4, 4, 4,},
            {5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 1, 0, 1, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 3, 3, 3, 3, 3,},
            {5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 1, 0, 1, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 2, 2, 2, 2,},
            {6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 1, 0, 1, 2, 5, 4, 3, 2, 1, 1, 1, 2,},
            {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 3, 3, 3, 3, 3, 3, 5, 4, 3, 2, 2, 2, 2, 2, 5, 4, 3, 2, 1, 1, 1, 2, 5, 4, 3, 2, 1, 0, 1, 2,},
            {6, 5, 4, 3, 2, 1, 0, 1, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 4, 4, 4, 4, 4, 6, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,},
            {6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 1, 0, 1, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 4, 4, 4, 4, 4, 6, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,},
            {6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 1, 0, 1, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 4, 4, 4, 4, 4, 6, 5, 5, 5, 5, 5, 5, 5,},
            {6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 1, 0, 1, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 4, 4, 4, 4, 4,},
            {6, 5, 4, 4, 4, 4, 4, 4, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 1, 0, 1, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 3, 3, 3, 3,},
            {6, 5, 5, 5, 5, 5, 5, 5, 6, 5, 4, 4, 4, 4, 4, 4, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 1, 0, 1, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 2, 2, 2,},
            {6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 6, 5, 4, 4, 4, 4, 4, 4, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 1, 0, 1, 6, 5, 4, 3, 2, 1, 1, 1,},
            {7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 6, 5, 4, 4, 4, 4, 4, 4, 6, 5, 4, 3, 3, 3, 3, 3, 6, 5, 4, 3, 2, 2, 2, 2, 6, 5, 4, 3, 2, 1, 1, 1, 6, 5, 4, 3, 2, 1, 0, 1,},
            {7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 4, 4, 4, 4, 7, 6, 5, 5, 5, 5, 5, 5, 7, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7,},
            {7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 4, 4, 4, 4, 7, 6, 5, 5, 5, 5, 5, 5, 7, 6, 6, 6, 6, 6, 6, 6,},
            {7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 4, 4, 4, 4, 7, 6, 5, 5, 5, 5, 5, 5,},
            {7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 4, 4, 4, 4,},
            {7, 6, 5, 4, 4, 4, 4, 4, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 3, 3, 3,},
            {7, 6, 5, 5, 5, 5, 5, 5, 7, 6, 5, 4, 4, 4, 4, 4, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 2, 2,},
            {7, 6, 6, 6, 6, 6, 6, 6, 7, 6, 5, 5, 5, 5, 5, 5, 7, 6, 5, 4, 4, 4, 4, 4, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 1,},
            {7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 7, 6, 5, 5, 5, 5, 5, 5, 7, 6, 5, 4, 4, 4, 4, 4, 7, 6, 5, 4, 3, 3, 3, 3, 7, 6, 5, 4, 3, 2, 2, 2, 7, 6, 5, 4, 3, 2, 1, 1, 7, 6, 5, 4, 3, 2, 1, 0,}
    };
}


