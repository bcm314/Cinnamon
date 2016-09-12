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

package main

import "fmt"
import "strings"

type _Tboard struct {
	allPieces            uint64;
	kingAttackers        [2]uint64;
	allPiecesSide        [2]uint64;
	openFile             uint64;
	semiOpenFile         [2] uint64;
	isolated             [2] uint64;
	allPiecesNoPawns     [2] uint64;
	kingSecurityDistance [2]int;
	posKing              [2] uint8;
}

const (
	PAWN_BLACK int = 0
	PAWN_WHITE int = 1
	ROOK_BLACK int = 2
	ROOK_WHITE int = 3
	BISHOP_BLACK int = 4
	BISHOP_WHITE int = 5
	KNIGHT_BLACK int = 6
	KNIGHT_WHITE int = 7
	KING_BLACK int = 8
	KING_WHITE int = 9
	QUEEN_BLACK int = 10
	QUEEN_WHITE int = 11

	RIGHT_CASTLE_IDX int = 12
	ENPASSANT_IDX int = 13
	SIDETOMOVE_IDX int = 14
	ZOBRISTKEY_IDX int = 15

	RIGHT_KING_CASTLE_WHITE_MASK uint8 = 0x10
	RIGHT_QUEEN_CASTLE_WHITE_MASK uint8 = 0x20
	RIGHT_KING_CASTLE_BLACK_MASK uint8 = 0x40
	RIGHT_QUEEN_CASTLE_BLACK_MASK uint8 = 0x80

	SQUARE_FREE int = 12

	NO_ENPASSANT uint64 = 100
	KING_SIDE_CASTLE_MOVE_MASK uint8 = 0x4
	QUEEN_SIDE_CASTLE_MOVE_MASK uint8 = 0x8

	A7bit uint64 = 0x80000000000000
	B7bit uint64 = 0x40000000000000
	C6bit uint64 = 0x200000000000
	H7bit uint64 = 0x1000000000000
	G7bit uint64 = 0x2000000000000
	F6bit uint64 = 0x40000000000
	A8bit uint64 = 0x8000000000000000
	H8bit uint64 = 0x100000000000000
	A2bit uint64 = 0x8000
	B2bit uint64 = 0x4000
	H2bit uint64 = 0x100
	G2bit uint64 = 0x200
	A1bit uint64 = 0x80
	H1bit uint64 = 0x1
	F1G1bit uint64 = 0x6
	H1H2G1bit uint64 = 0x103
	C1B1bit uint64 = 0x60
	A1A2B1bit uint64 = 0x80c0
	F8G8bit uint64 = 0x600000000000000
	H8H7G8bit uint64 = 0x301000000000000
	C8B8bit uint64 = 0x6000000000000000
	A8A7B8bit uint64 = 0xc080000000000000
	C6A6bit uint64 = 0xa00000000000
	F6H6bit uint64 = 0x50000000000
	A7C7bit uint64 = 0xa0000000000000
	H7G7bit uint64 = 0x3000000000000
	C3A3bit uint64 = 0xa00000
	F3H3bit uint64 = 0x50000
	A2C2bit uint64 = 0xa000
	H2G2bit uint64 = 0x300

	E1 int = 3
	E8 int = 59
	C1 int = 5
	F1 int = 2
	C8 int = 61
	F8 int = 58
	D8 int = 60
	A8 int = 63
	H8 int = 56
	G8 int = 57
	BLACK_SQUARES uint64 = 0x55AA55AA55AA55AA
	WHITE_SQUARES uint64 = 0xAA55AA55AA55AA55
)

type  ChessBoard struct {
	Bitboard
	chessboard    _Tchessboard
	fenString     string
	structureEval _Tboard
}

func ( self ChessBoard ) decodeBoardinv(typ uint8, a int, side int) string {
	if (typ & QUEEN_SIDE_CASTLE_MOVE_MASK != 0&& side == WHITE) {
		return "e1c1";
	}
	if (typ & KING_SIDE_CASTLE_MOVE_MASK != 0&&  side == WHITE) {
		return "e1g1";
	}
	if (typ & QUEEN_SIDE_CASTLE_MOVE_MASK != 0&&  side == BLACK) {
		return "e8c8";
	}
	if (typ & KING_SIDE_CASTLE_MOVE_MASK != 0&&  side == BLACK) {
		return "e8g8";
	}
	if ((typ & 0xC) == 0) {
		for {
			fmt.Printf("error")
		}
	}
	if (a >= 0 && a < 64) {
		return BOARD[a];
	}
	for {
		fmt.Printf("error")
	}
}

func ( self ChessBoard ) display() {
	fmt.Printf("\n     a   b   c   d   e   f   g   h");
	for t := 0; t <= 63; t++ {
		var x = ' ';
		if (t % 8 == 0) {
			fmt.Printf("\n   ----+---+---+---+---+---+---+----\n");
			fmt.Printf(" %v | ", ( 8 - RANK_AT[t] ));
		}
		var o = FEN_PIECE[self.getPieceAt(WHITE, POW2[63 - t])];

		var k = ' ';
		if o != '-' {
			k = rune(o)
		} else {
			k = FEN_PIECE[self.getPieceAt(BLACK, POW2[63 - t])]
		};
		if k == '-' {
			x = ' '
		} else {
			x = k
		};

		if x != ' ' {
			fmt.Printf("%v", x); } else {
			if POW2[t] & WHITE_SQUARES != 0 {
				fmt.Printf(" "); } else {
				fmt.Printf(".")
			};
		}
		fmt.Printf(" | ");
	};
	fmt.Printf("\n   ----+---+---+---+---+---+---+----\n");
	//fmt.Printf("     a   b   c   d   e   f   g   h\n\n\n{}\n\n", self.boardToFen());


	fmt.Printf("zobristKey: %v", self.chessboard[ZOBRISTKEY_IDX]);
	fmt.Printf("enpassantPosition: %v", self.chessboard[ENPASSANT_IDX]);
	fmt.Printf("rightCastle: %v", self.chessboard[RIGHT_CASTLE_IDX]);
	fmt.Printf("sideToMove: %v", self.chessboard[SIDETOMOVE_IDX]);
}

func ( self ChessBoard ) getFen() string {
	return self.fenString;
}

//char decodeBoard(string);

func ( self ChessBoard )loadFen(fen string) int {
	var iss = strings.Split(fen, " ");
	var pos = iss[0];
	var side = iss[1];
	var castle = iss[2];
	var enpassant = iss[3];
	var ix = 0;
	s := make([]uint, 64);
	//var s: [usize; 64] = [0; 64];
	//let chars: Vec<char> = pos.chars().collect();
	for ch := range pos {
		if ch != '/' {
			if INV_FEN[ch ] != 0xFF {
				s[ix] = INV_FEN[ch                                ];
				ix = ix + 1;
			} else if ch > 47 && ch < 58 {
				dummy := 0;
				for dummy < int(ch) - 48 {
					dummy++;
					s[ix] = SQUARE_FREE;
					ix = ix + 1;
				}
			} else {
				return 2;
			};
		}
	}
	if ix != 64 {
		return 2
	}
	if side == "b" {
		self.chessboard[SIDETOMOVE_IDX] = BLACK;

	} else if side == "w" {
		self.chessboard[SIDETOMOVE_IDX] = WHITE;
	} else {
		return 2
	}
	i := 0;
	for i < 64 {
		p := s[63 - i];
		if p != SQUARE_FREE {
			self.updateZobristKey(p, i);
			self.chessboard[p] |= POW2[i];
		} else {
			self.chessboard[p] &= NOTPOW2[i];
		}
	};

	let
	cast: Vec < char > = castle.chars().collect();
	for
	ch
	in
	cast{
	if ch == 'K' {
		&self.update_zobrist_key(RIGHT_CASTLE_IDX, 4);
		assert
		!((4 == RIGHT_KING_CASTLE_WHITE_MASK.bitscan_forward()));
		self.chessboard[RIGHT_CASTLE_IDX] |= self.chessboard[RIGHT_CASTLE_IDX];
	} else
	if ch == 'k' {
		&self.update_zobrist_key(RIGHT_CASTLE_IDX, 6);
		assert
		!((6 == RIGHT_KING_CASTLE_BLACK_MASK.bitscan_forward()));
		self.chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_BLACK_MASK;
	} else
	if ch == 'Q' {
		&self.update_zobrist_key(RIGHT_CASTLE_IDX, 5);
		assert
		!((5 == RIGHT_QUEEN_CASTLE_WHITE_MASK.bitscan_forward()));
		self.chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
	} else
	if ch == 'q' {
		&self.update_zobrist_key(RIGHT_CASTLE_IDX, 7);
		assert
		!((7 == RIGHT_QUEEN_CASTLE_BLACK_MASK.bitscan_forward()));
		self.chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_BLACK_MASK;
	}
};
self.chessboard[ENPASSANT_IDX] = NO_ENPASSANT;

for i in 0..64 {
if enpassant == BOARD[i] {
self.chessboard[ENPASSANT_IDX] = i as u64;

if self.chessboard[SIDETOMOVE_IDX] != 0 {
self.chessboard[ENPASSANT_IDX] -= 8;
} else {
self.chessboard[ENPASSANT_IDX] += 8;
}
&self.update_zobrist_key(ENPASSANT_IDX, self.chessboard[ENPASSANT_IDX] as usize);
break;
}
}
self.chessboard[SIDETOMOVE_IDX] as i32
return 1;
}

//int getPieceByChar(char);


func ( self ChessBoard ) getBitmap(side int) uint64 {
	return self.chessboard[PAWN_BLACK + side] | self.chessboard[ROOK_BLACK + side] | self.chessboard[BISHOP_BLACK + side] | self.chessboard[KNIGHT_BLACK + side] | self.chessboard[KING_BLACK + side] | self.chessboard[QUEEN_BLACK + side];
}

func ( self ChessBoard )  setSide(b uint64) {
	self.chessboard[SIDETOMOVE_IDX] = b;
}

func ( self ChessBoard )   getSide() int {
	return int(self.chessboard[SIDETOMOVE_IDX]);
}

func ( self ChessBoard )  getBitmapNoPawns(side int) uint64 {
	return self.chessboard[ROOK_BLACK + side] | self.chessboard[BISHOP_BLACK + side] | self.chessboard[KNIGHT_BLACK + side] | self.chessboard[KING_BLACK + side] | self.chessboard[QUEEN_BLACK + side];
}


//void makeZobristKey();
//
//template<int side>
//int getNpiecesNoPawnNoKing() const {
//return bitCount(chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] | chessboard[QUEEN_BLACK + side]);
//}


func ( self ChessBoard ) getPieceAt(side int, bitmapPos uint64) int {
	if self.chessboard[PAWN_BLACK + side] & bitmapPos != 0 {
		return PAWN_BLACK + side; }
	if self.chessboard[ROOK_BLACK + side] & bitmapPos != 0 {
		return ROOK_BLACK + side
	}
	if self.chessboard[BISHOP_BLACK + side] & bitmapPos != 0 {
		return BISHOP_BLACK + side
	}
	if self.chessboard[KNIGHT_BLACK + side] & bitmapPos != 0 {
		return KNIGHT_BLACK + side
	}
	if self.chessboard[QUEEN_BLACK + side] & bitmapPos != 0 {
		return QUEEN_BLACK + side
	}
	if self.chessboard[KING_BLACK + side] & bitmapPos != 0 {
		return KING_BLACK + side
	}
	return SQUARE_FREE;
}

func ( self ChessBoard )updateZobristKey(piece uint, position uint) {
	self.chessboard[ZOBRISTKEY_IDX] = self.chessboard[ZOBRISTKEY_IDX] ^ RANDOM_KEY[piece][position];
}