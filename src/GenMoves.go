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

const (
	MAX_MOVE int = 130
	RANK_1 uint64 = 0xff00
	RANK_3 uint64 = 0xff000000
	RANK_4 uint64 = 0xff00000000
	RANK_6 uint64 = 0xff000000000000
	STANDARD_MOVE_MASK uint64 = 0x3
	ENPASSANT_MOVE_MASK uint64 = 0x1
	PROMOTION_MOVE_MASK uint64 = 0x2
	MAX_REP_COUNT int = 1024
	NO_PROMOTION int = -1

	TABJUMPPAWN uint64 = 0xFF00000000FF00;
	TABCAPTUREPAWN_RIGHT uint64 = 0xFEFEFEFEFEFEFEFE;
	TABCAPTUREPAWN_LEFT uint64 = 0x7F7F7F7F7F7F7F7F;


)

type  GenMoves struct {
	chessBoard *ChessBoard
	perftMode  bool
	listId     int
	gen_list   []_TmoveP
	currentPly int
	numMoves   uint64
	numMovesq  uint64
	forceCheck bool
}

func ( self *GenMoves )  setPerft(b bool) {
	self.perftMode = b;
}

func NewGenMoves() *GenMoves {
	fmt.Print("NewGenMoves\n")
	p := new(GenMoves)
	p.gen_list = make([]_TmoveP, MAX_PLY)
	p.chessBoard = NewChessBoard()
	return p
}

func ( self *GenMoves ) generateMoves(side uint, allpieces uint64) {

	self.tryAllCastle(side, allpieces);
	self.performDiagShift(BISHOP_BLACK + side, side, allpieces);
	self.performRankFileShift(ROOK_BLACK + side, side, allpieces);
	self.performRankFileShift(QUEEN_BLACK + side, side, allpieces);
	self.performDiagShift(QUEEN_BLACK + side, side, allpieces);
	self.performPawnShift(side, ^allpieces );
	self.performKnightShiftCapture(KNIGHT_BLACK + side, ^allpieces , side);
	self.performKingShiftCapture(side, ^allpieces );
}

func ( self *GenMoves )  generateCaptures(side uint, enemies uint64, friends uint64) bool {

	var allpieces = enemies | friends;
	if self.performPawnCapture(enemies, side) {
		return true;
	}
	if self.performKingShiftCapture(side, enemies) {
		return true;
	}
	if self.performKnightShiftCapture(KNIGHT_BLACK + side, enemies, side) {
		return true;
	}
	if self.performDiagCapture(BISHOP_BLACK + side, enemies, side, allpieces) {
		return true;
	}
	if self.performRankFileCapture(ROOK_BLACK + side, enemies, side, allpieces) {
		return true;
	}
	if self.performRankFileCapture(QUEEN_BLACK + side, enemies, side, allpieces) {
		return true;
	}
	if self.performDiagCapture(QUEEN_BLACK + side, enemies, side, allpieces) {
		return true;
	}
	return false;
}

func ( self *GenMoves ) getForceCheck() bool {
	return self.forceCheck;
}

func ( self *GenMoves ) setForceCheck(b bool) {
	self.forceCheck = b;
}

func ( self *GenMoves ) init() {
	self.numMoves = 0;
	self.numMovesq = 0;
	self.listId = 0;
	//        #ifdef DEBUG_MODE
	//        nCutFp = nCutRazor = 0;
	//        betaEfficiency = 0.0;
	//        nCutAB = 0;
	//        nNullMoveCut = 0;
	//        #endif
}

func ( self *GenMoves ) loadFen(fen string) int {
	//self.repetitionMapCount = 0;
	var side = self.chessBoard.loadFen(fen);
	if side == 2 {
		panic("Bad FEN position format ");
	}
	return side;
}

func ( self *GenMoves ) getDiagCapture(position uint, allpieces uint64, enemies uint64) uint64 {
	return self.chessBoard.bitboard.getDiagonalAntiDiagonal(position, allpieces) & enemies;
}

func ( self *GenMoves ) getDiagShiftAndCapture(position uint, enemies uint64, allpieces uint64) uint64 {
	var nuovo = self.chessBoard.bitboard.getDiagonalAntiDiagonal(position, allpieces);
	return (nuovo & enemies) | (nuovo & (^allpieces));
}

func ( self *GenMoves ) takeback(mov *_Tmove, oldkey uint64, rep bool) {
	//if rep {
	//	self.popStackMove();
	//}
	self.chessBoard.chessboard[ZOBRISTKEY_IDX] = oldkey;
	self.chessBoard.chessboard[ENPASSANT_IDX] = NO_ENPASSANT;

	var pieceFrom uint;
	var posTo uint;
	var posFrom uint;
	var movecapture uint;
	self.chessBoard.chessboard[RIGHT_CASTLE_IDX] = uint64((mov.typee & 0xf0));
	if (mov.typee & 0x3) == STANDARD_MOVE_MASK || (mov.typee & 0x3) == ENPASSANT_MOVE_MASK {
		posTo = mov.to;
		posFrom = mov.from;
		movecapture = mov.capturedPiece;

		pieceFrom = mov.pieceFrom;
		self.chessBoard.chessboard[pieceFrom] = (self.chessBoard.chessboard[pieceFrom] & NOTPOW2[posTo]) | POW2[posFrom];
		if movecapture != SQUARE_FREE {
			if (mov.typee & 0x3) != ENPASSANT_MOVE_MASK {
				self.chessBoard.chessboard[movecapture] |= POW2[posTo ];
			} else {

				if mov.side != 0 {
					self.chessBoard.chessboard[movecapture] |= POW2[(posTo - 8)];
				} else {
					self.chessBoard.chessboard[movecapture] |= POW2[(posTo + 8)];
				}
			}
		}
	} else if (mov.typee & 0x3) == PROMOTION_MOVE_MASK {
		posTo = mov.to;
		posFrom = mov.from;
		movecapture = mov.capturedPiece;

		self.chessBoard.chessboard[mov.side] |= POW2[posFrom];
		self.chessBoard.chessboard[mov.promotionPiece] &= NOTPOW2[posTo];
		if movecapture != SQUARE_FREE {
			self.chessBoard.chessboard[movecapture] |= POW2[posTo];
		}
	} else if mov.typee & 0xc != 0 {
		//castle
		self.unPerformCastle(mov.side, mov.typee);
	}
}

func ( self *GenMoves )  getDiagShiftCount(position uint, allpieces uint64) uint {

	return bitCount((self.chessBoard.bitboard.getDiagonalAntiDiagonal(position, allpieces) & ^allpieces));
}

func ( self *GenMoves )  performKingShiftCapture(side uint, enemies uint64) bool {

	var pos = BITScanForward(self.chessBoard.chessboard[KING_BLACK + side]);

	var x1 = enemies & NEAR_MASK1[pos];
	for ; x1 != 0; {
		if self.pushmove(STANDARD_MOVE_MASK, pos, BITScanForward(x1), side, NO_PROMOTION, (KING_BLACK + side)) {
			return true;
		}
		x1 = reset_lsb(x1);
	};
	return false;
}

func ( self *GenMoves )  performKnightShiftCapture(piece uint, enemies uint64, side uint) bool {

	var x = self.chessBoard.chessboard[piece];
	for ; x != 0; {
		var pos = BITScanForward(x);
		var x1 = enemies & KNIGHT_MASK[pos];
		for ; x1 != 0; {
			if self.pushmove(STANDARD_MOVE_MASK, pos, BITScanForward(x1), side, NO_PROMOTION, piece) {
				return true;
			}
			x1 = reset_lsb(x1);
		};
		x = reset_lsb(x);
	}
	return false;
}

func ( self *GenMoves ) performDiagCapture(piece uint, enemies uint64, side uint, allpieces uint64) bool {

	var x2 = self.chessBoard.chessboard[piece];
	for ; x2 != 0; {
		var position = BITScanForward(x2);
		var diag = self.chessBoard.bitboard.getDiagonalAntiDiagonal(position, allpieces) & enemies;

		for ; diag != 0; {
			if self.pushmove(STANDARD_MOVE_MASK, position, BITScanForward(diag), side, NO_PROMOTION, piece) {
				return true;
			}
			diag = reset_lsb(diag);
		}
		x2 = reset_lsb(x2);
	}
	return false;
}

func ( self *GenMoves ) getTotMoves() uint64 {
	return self.numMoves + self.numMovesq;
}

func ( self *GenMoves ) performRankFileCapture(piece uint, enemies uint64, side uint, allpieces uint64) bool {

	var x2 = self.chessBoard.chessboard[piece];
	for ; x2 != 0; {
		var position = BITScanForward(x2);
		var rankFile = self.chessBoard.bitboard.getRankFile(position, allpieces) & enemies;
		for ; rankFile != 0; {
			if self.pushmove(STANDARD_MOVE_MASK, position, BITScanForward(rankFile), side, NO_PROMOTION, piece) {
				return true;
			}
			rankFile = reset_lsb(rankFile);
		}
		x2 = reset_lsb(x2);
	}
	return false;
}

func ( self *GenMoves ) performPawnCapture(enemies uint64, side uint) bool {
	if self.chessBoard.chessboard[side] == 0 {
		if self.chessBoard.chessboard[ENPASSANT_IDX] != NO_ENPASSANT {
			self.chessBoard.updateZobristKey(13, uint(self.chessBoard.chessboard[ENPASSANT_IDX]));
		}
		self.chessBoard.chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
		return false;
	}
	var GG int
	var x uint64
	if side != 0 {
		x = (self.chessBoard.chessboard[side] << 7) & TABCAPTUREPAWN_LEFT & enemies;
		GG = -7;
	} else {
		x = (self.chessBoard.chessboard[side] >> 7) & TABCAPTUREPAWN_RIGHT & enemies;
		GG = 7;
	};
	for ; x != 0; {
		var o = BITScanForward(x);
		var xx = uint(int(o) + GG);
		if (side != 0 && o > 55) || (side == 0 && o < 8) {
			//PROMOTION

			if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(QUEEN_BLACK + side), side) {
				return true; //queen
			}
			if self.perftMode == true {
				if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(KNIGHT_BLACK + side), side) {
					return true; //knight
				}
				if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(ROOK_BLACK + side), side) {
					return true; //rock
				}
				if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(BISHOP_BLACK + side), side) {
					return true; //bishop
				}
			}
		} else if self.pushmove(STANDARD_MOVE_MASK, xx, o, side, NO_PROMOTION, side) {
			return true;
		}
		x = reset_lsb(x);
	};
	if side != 0 {
		GG = -9;
		x = (self.chessBoard.chessboard[side] << 9) & TABCAPTUREPAWN_RIGHT & enemies;
	} else {
		GG = 9;
		x = (self.chessBoard.chessboard[side] >> 9) & TABCAPTUREPAWN_LEFT & enemies;
	};
	for ; x != 0; {
		var o = BITScanForward(x);
		var xx = uint(int(o) + GG);
		if (side != 0 && o > 55) || (side == 0 && o < 8) {
			//PROMOTION

			if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(QUEEN_BLACK + side), side) {
				return true; //queen
			}
			if self.perftMode == true {
				if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(KNIGHT_BLACK + side), side) {
					return true; //knight
				}
				if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(BISHOP_BLACK + side), side) {
					return true; //bishop
				}
				if self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(ROOK_BLACK + side), side) {
					return true; //rock
				}
			}
		} else if self.pushmove(STANDARD_MOVE_MASK, xx, o, side, NO_PROMOTION, side) {
			return true;
		}
		x = reset_lsb(x);
	};
	//ENPASSANT
	if self.chessBoard.chessboard[ENPASSANT_IDX] != NO_ENPASSANT {
		x = ENPASSANT_MASK[side ^ 1][self.chessBoard.chessboard[ENPASSANT_IDX]] & self.chessBoard.chessboard[side];
		for ; x != 0; {
			var o = BITScanForward(x);
			var ff uint64
			if side != 0 {
				ff = self.chessBoard.chessboard[ENPASSANT_IDX] + 8
			} else {
				ff = self.chessBoard.chessboard[ENPASSANT_IDX] - 8
			};
			self.pushmove(ENPASSANT_MOVE_MASK, o, uint(ff), side, NO_PROMOTION, side);
			x = reset_lsb(x);
		}
		self.chessBoard.updateZobristKey(13, uint(self.chessBoard.chessboard[ENPASSANT_IDX]));
		self.chessBoard.chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
	}
	return false;
}

func ( self *GenMoves ) performPawnShift(side uint, xallpieces uint64) {
	var tt int
	var x = self.chessBoard.chessboard[side];
	if x & PAWNS_JUMP[side] != 0 {
		self.checkJumpPawn(side, x, xallpieces);
	}
	if side != 0 {
		x <<= 8;
		tt = -8;
	} else {
		tt = 8;
		x >>= 8;
	};
	x &= xallpieces;
	for ; x != 0; {
		var o = BITScanForward(x);
		var xx = uint(int(o) + tt);
		if o > 55 || o < 8 {
			self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(QUEEN_BLACK + side), side);
			if self.perftMode == true {
				self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(KNIGHT_BLACK + side), side);
				self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(BISHOP_BLACK + side), side);
				self.pushmove(PROMOTION_MOVE_MASK, xx, o, side, int(ROOK_BLACK + side), side);
			}
		} else {
			self.pushmove(STANDARD_MOVE_MASK, xx, o, side, NO_PROMOTION, side);
		}
		x = reset_lsb(x);
	};
}

func ( self *GenMoves ) clearKillerHeuristic() {
	//	self.killerHeuristic = [[0; 64]; 64];
	//memset(killerHeuristic, 0, sizeof (killerHeuristic));
}

func ( self *GenMoves ) performDiagShift(piece uint, side uint, allpieces uint64) {

	var x2 = self.chessBoard.chessboard[piece];
	for ; x2 != 0; {
		var position = BITScanForward(x2);
		var diag = self.chessBoard.bitboard.getDiagonalAntiDiagonal(position, allpieces) & ^allpieces;
		for ; diag != 0; {
			self.pushmove(STANDARD_MOVE_MASK, position, BITScanForward(diag), side, NO_PROMOTION, piece);
			diag = reset_lsb(diag);
		}
		x2 = reset_lsb(x2);
	}
}

func ( self *GenMoves ) performRankFileShift(piece uint, side uint, allpieces uint64) {

	var x2 = self.chessBoard.chessboard[piece];
	for ; x2 != 0; {
		var position = BITScanForward(x2);
		var rankFile = self.chessBoard.bitboard.getRankFile(position, allpieces) & ^allpieces;
		for ; rankFile != 0; {
			self.pushmove(STANDARD_MOVE_MASK, position, BITScanForward(rankFile), side, NO_PROMOTION, piece);
			rankFile = reset_lsb(rankFile);

		}
		x2 = reset_lsb(x2);
	}
}

func ( self *GenMoves ) makemove(mov *_Tmove, rep bool, checkInCheck bool) bool {
	var pieceFrom uint = SQUARE_FREE;
	var posTo uint;
	var posFrom uint;
	var movecapture = SQUARE_FREE;
	var rightCastleOld = self.chessBoard.chessboard[RIGHT_CASTLE_IDX];
	if mov.typee & 0xc == 0 {
		//no castle
		posTo = mov.to;
		posFrom = mov.from;
		movecapture = mov.capturedPiece;

		pieceFrom = mov.pieceFrom;
		if (mov.typee & 0x3) == PROMOTION_MOVE_MASK {
			self.chessBoard.chessboard[pieceFrom] &= NOTPOW2[posFrom];
			self.chessBoard.updateZobristKey(pieceFrom, posFrom);

			self.chessBoard.chessboard[mov.promotionPiece] |= POW2[posTo];
			self.chessBoard.updateZobristKey(uint(mov.promotionPiece), posTo);
		} else {
			self.chessBoard.chessboard[pieceFrom] = (self.chessBoard.chessboard[pieceFrom] | POW2[posTo]) & NOTPOW2[posFrom];
			self.chessBoard.updateZobristKey(pieceFrom, posFrom);
			self.chessBoard.updateZobristKey(pieceFrom, posTo);
		}
		if movecapture != SQUARE_FREE {
			if (mov.typee & 0x3) != ENPASSANT_MOVE_MASK {
				self.chessBoard.chessboard[movecapture] &= NOTPOW2[posTo];
				self.chessBoard.updateZobristKey(movecapture, posTo);
			} else {
				//en passant

				if mov.side != 0 {
					self.chessBoard.chessboard[movecapture] &= NOTPOW2[posTo - 8];
					self.chessBoard.updateZobristKey(movecapture, posTo - 8);
				} else {
					self.chessBoard.chessboard[movecapture] &= NOTPOW2[posTo + 8];
					self.chessBoard.updateZobristKey(movecapture, posTo + 8);
				}
			}
		}
		//lost castle right
		switch pieceFrom{
		case KING_WHITE :{
			self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0xcf;
		}

		case KING_BLACK :{
			self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0x3f;
		}
		case ROOK_WHITE :{
			if posFrom == 0 {
				self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0xef;
			} else if posFrom == 7 {
				self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0xdf;
			}
		}
		case ROOK_BLACK :{
			if posFrom == 56 {
				self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0xbf;
			} else if posFrom == 63 {
				self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0x7f;
			}
		}
		//en passant

		case PAWN_WHITE :{
			if (RANK_1 & POW2[posFrom]) != 0 && (RANK_3 & POW2[posTo]) != 0 {
				self.chessBoard.chessboard[ENPASSANT_IDX] = uint64(posTo);
				self.chessBoard.updateZobristKey(13, uint(self.chessBoard.chessboard[ENPASSANT_IDX]));
			}
		}

		case PAWN_BLACK :{
			if (RANK_6 & POW2[posFrom]) != 0 && (RANK_4 & POW2[posTo]) != 0 {
				self.chessBoard.chessboard[ENPASSANT_IDX] = uint64(posTo);
				self.chessBoard.updateZobristKey(13, uint(self.chessBoard.chessboard[ENPASSANT_IDX]));
			}
		}

		}
	} else {
		//castle
		self.performCastle(mov.side, mov.typee);
		if mov.side == WHITE {
			self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0xcf;
		} else {
			self.chessBoard.chessboard[RIGHT_CASTLE_IDX] &= 0x3f;
		}
	}
	var x2 = rightCastleOld ^ self.chessBoard.chessboard[RIGHT_CASTLE_IDX];
	for ; x2 != 0; {
		var position = BITScanForward(x2);
		self.chessBoard.updateZobristKey(14, position);
		x2 = reset_lsb(x2);
	}
	//if rep == true {
	//	if movecapture != SQUARE_FREE || pieceFrom == WHITE || pieceFrom == BLACK || mov.typee & 0xcuint64 != 0 {
	//		self.pushStackMove1(0);
	//	}
	//	self.pushStackMove1(self.chessBoard.chessboard[ZOBRISTKEY_IDX]);
	//}
	if (self.forceCheck || (checkInCheck == true && self.perftMode == false)) && ((mov.side == WHITE && self.inCheck1(WHITE) != 0) || (mov.side == BLACK && self.inCheck1(BLACK) != 0)) {
		return false;
	}
	return true;
}

func ( self *GenMoves ) incListId() {
	self.listId = self.listId + 1;
}

func ( self *GenMoves ) display() {
	self.chessBoard.display();
}

func ( self *GenMoves ) decListId() {

	self.gen_list[self.listId].size = 0;
	self.listId = self.listId - 1;
}

func ( self *GenMoves ) getListSize() int {
	return self.gen_list[self.listId].size;
}

func ( self *GenMoves ) resetList() {
	self.gen_list[self.listId].size = 0;
}

func ( self *GenMoves ) getNextMove(list _TmoveP) *_Tmove {
	//var gen_list1 = &list.moveList;

	var listcount uint = uint(list.size);
	var bestId uint = 9999999;

	var bestScore int;
	var j uint;
	for j := 0; j < int(listcount); j++ {
		if list.moveList[j].used == false {
			bestId = uint(j);
			bestScore = list.moveList[uint(bestId)].score;
			break;
		}
	}
	if bestId == 9999999 {
		return nil;
	}
	for i := j + 1; i < listcount; i++ {
		if list.moveList[i].used == true && list.moveList[i].score > bestScore {
			bestId = i;
			bestScore = list.moveList[bestId].score;
		}
	}
	list.moveList[bestId].used = true;
	return &list.moveList[bestId];
}

func ( self *GenMoves ) isAttacked(side uint, position uint, allpieces uint64) uint64 {
	return self.getAttackers(side, true, position, allpieces)
}

func ( self *GenMoves ) getAllAttackers(side uint, position uint, allpieces uint64) uint64 {
	return self.getAttackers(side, false, position, allpieces)
}

func ( self *GenMoves ) getMobilityRook(position uint, enemies uint64, friends uint64) uint {

	return self.performRankFileCaptureAndShiftCount(position, enemies, enemies | friends)
}

func ( self *GenMoves ) getMobilityPawns(side uint, ep uint64, ped_friends uint64, enemies uint64, xallpieces uint64) uint {

	if ep == NO_ENPASSANT {
		return 0
	}
	if bitCount((ENPASSANT_MASK[side ^ 1][ep] & self.chessBoard.chessboard[side])) + side == WHITE {
		return bitCount(((ped_friends << 8) & xallpieces)) + bitCount(((((ped_friends & TABJUMPPAWN) << 8) & xallpieces) << 8) & xallpieces) + bitCount((self.chessBoard.chessboard[side] << 7) & TABCAPTUREPAWN_LEFT & enemies) + bitCount((self.chessBoard.chessboard[side] << 9) & TABCAPTUREPAWN_RIGHT & enemies)
	} else {
		return bitCount(((ped_friends >> 8) & xallpieces)) + bitCount(((((ped_friends & TABJUMPPAWN) >> 8) & xallpieces) >> 8) & xallpieces) + bitCount((self.chessBoard.chessboard[side] >> 7) & TABCAPTUREPAWN_RIGHT & enemies) + bitCount((self.chessBoard.chessboard[side] >> 9) & TABCAPTUREPAWN_LEFT & enemies);
	}
}

func ( self *GenMoves ) getMobilityCastle(side uint, allpieces uint64) int {

	var count = 0;
	if side == WHITE {
		if POW2_3 & self.chessBoard.chessboard[KING_WHITE] != 0 && (allpieces & 0x6) == 0 && self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK != 0 && self.chessBoard.chessboard[ROOK_WHITE] & POW2_0 != 0 && self.isAttacked(WHITE, 1, allpieces) == 0 && self.isAttacked(WHITE, 2, allpieces) == 0 && self.isAttacked(WHITE, 3, allpieces) == 0 {
			count = count + 1;
		}
		if POW2_3 & self.chessBoard.chessboard[KING_WHITE] != 0 && (allpieces & 0x70) == 0 && self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK != 0 && self.chessBoard.chessboard[ROOK_WHITE] & POW2_7 != 0 && self.isAttacked(WHITE, 3, allpieces) == 0 && self.isAttacked(WHITE, 4, allpieces) == 0 && self.isAttacked(WHITE, 5, allpieces) == 0 {
			count = count + 1;
		}
	} else {
		if POW2_59 & self.chessBoard.chessboard[KING_BLACK] != 0 && self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK != 0 && (allpieces & 0x600000000000000) == 0 && self.chessBoard.chessboard[ROOK_BLACK] & POW2_56 != 0 && self.isAttacked(BLACK, 57, allpieces) == 0 && self.isAttacked(BLACK, 58, allpieces) == 0 && self.isAttacked(BLACK, 59, allpieces) == 0 {
			count = count + 1;
		}
		if POW2_59 & self.chessBoard.chessboard[KING_BLACK] != 0 && self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK != 0 && (allpieces & 0x7000000000000000) == 0 && self.chessBoard.chessboard[ROOK_BLACK] & POW2_63 != 0 && self.isAttacked(BLACK, 59, allpieces) == 0 && self.isAttacked(BLACK, 60, allpieces) == 0 && self.isAttacked(BLACK, 61, allpieces) == 0 {
			count = count + 1;
		}
	}
	return count;
}

func ( self *GenMoves ) getMobilityQueen(position uint, enemies uint64, allpieces uint64) uint {

	return self.performRankFileCaptureAndShiftCount(position, enemies, allpieces) +
		bitCount(self.getDiagShiftAndCapture(position, enemies, allpieces))
}

func ( self *GenMoves ) inCheck(side uint, typee uint64, from uint, to uint, pieceFrom uint, pieceTo uint, promotionPiece uint) uint64 {

	var result uint64;
	var g3 uint64 = uint64(typee & 0x3);
	switch  g3{
	case
		STANDARD_MOVE_MASK :{
		var from1 = self.chessBoard.chessboard[pieceFrom];
		var to1 uint64 = 0xffffffffffffffff;


		if pieceTo != SQUARE_FREE {
			to1 = self.chessBoard.chessboard[pieceTo];
			self.chessBoard.chessboard[pieceTo] &= NOTPOW2[to];
		}
		self.chessBoard.chessboard[pieceFrom] &= NOTPOW2[from];
		self.chessBoard.chessboard[pieceFrom] |= POW2[to];

		result = self.isAttacked(side, BITScanForward(self.chessBoard.chessboard[KING_BLACK + side]), self.chessBoard.getBitmap(BLACK) | self.chessBoard.getBitmap(WHITE));
		self.chessBoard.chessboard[pieceFrom] = from1;
		if pieceTo != SQUARE_FREE {
			self.chessBoard.chessboard[pieceTo] = to1;
		}
	}

	case PROMOTION_MOVE_MASK :{
		var to1 uint64 = 0;
		if pieceTo != SQUARE_FREE {
			to1 = self.chessBoard.chessboard[pieceTo];
		}
		var from1 uint64 = self.chessBoard.chessboard[pieceFrom];
		var p1 = self.chessBoard.chessboard[promotionPiece];
		self.chessBoard.chessboard[pieceFrom] &= NOTPOW2[from];
		if pieceTo != SQUARE_FREE {
			self.chessBoard.chessboard[pieceTo] &= NOTPOW2[to];
		}
		self.chessBoard.chessboard[promotionPiece] = self.chessBoard.chessboard[promotionPiece] | POW2[to];
		result = self.isAttacked(side, BITScanForward(self.chessBoard.chessboard[KING_BLACK + side]), self.chessBoard.getBitmap(BLACK) | self.chessBoard.getBitmap(WHITE));
		if pieceTo != SQUARE_FREE {
			self.chessBoard.chessboard[pieceTo] = to1;
		}
		self.chessBoard.chessboard[pieceFrom] = from1;
		self.chessBoard.chessboard[promotionPiece] = p1;
	}

	case ENPASSANT_MOVE_MASK :{
		var to1 = self.chessBoard.chessboard[side ^ 1];
		var from1 = self.chessBoard.chessboard[side];
		self.chessBoard.chessboard[side] &= NOTPOW2[from];
		self.chessBoard.chessboard[side] |= POW2[to];
		if side != 0 {
			self.chessBoard.chessboard[side ^ 1] &= NOTPOW2[to - 8];
		} else {
			self.chessBoard.chessboard[side ^ 1] &= NOTPOW2[to + 8];
		}
		result = self.isAttacked(side, BITScanForward(self.chessBoard.chessboard[KING_BLACK + side]), self.chessBoard.getBitmap(BLACK) | self.chessBoard.getBitmap(WHITE));
		self.chessBoard.chessboard[side ^ 1] = to1;
		self.chessBoard.chessboard[side] = from1;
	}

	}

	return result;
}

func ( self *GenMoves ) performCastle(side uint, typee uint64) {

	if side == WHITE {
		if typee & KING_SIDE_CASTLE_MOVE_MASK != 0 {

			self.chessBoard.updateZobristKey(KING_WHITE, 3);
			self.chessBoard.updateZobristKey(KING_WHITE, 1);
			self.chessBoard.chessboard[KING_WHITE] = self.chessBoard.chessboard[KING_WHITE] | POW2_1 & NOTPOW2_3;
			self.chessBoard.updateZobristKey(ROOK_WHITE, 2);
			self.chessBoard.updateZobristKey(ROOK_WHITE, 0);
			self.chessBoard.chessboard[ROOK_WHITE] = self.chessBoard.chessboard[ROOK_WHITE] | POW2_2 & NOTPOW2_0;
		} else {

			self.chessBoard.chessboard[KING_WHITE] = self.chessBoard.chessboard[KING_WHITE] | POW2_5 & NOTPOW2_3;
			self.chessBoard.updateZobristKey(KING_WHITE, 5);
			self.chessBoard.updateZobristKey(KING_WHITE, 3);
			self.chessBoard.chessboard[ROOK_WHITE] = self.chessBoard.chessboard[ROOK_WHITE] | POW2_4 & NOTPOW2_7;
			self.chessBoard.updateZobristKey(ROOK_WHITE, 4);
			self.chessBoard.updateZobristKey(ROOK_WHITE, 7);
		}
	} else {
		if typee & KING_SIDE_CASTLE_MOVE_MASK != 0 {

			self.chessBoard.chessboard[KING_BLACK] = self.chessBoard.chessboard[KING_BLACK] | POW2_57 & NOTPOW2_59;
			self.chessBoard.updateZobristKey(KING_BLACK, 57);
			self.chessBoard.updateZobristKey(KING_BLACK, 59);
			self.chessBoard.chessboard[ROOK_BLACK] = self.chessBoard.chessboard[ROOK_BLACK] | POW2_58 & NOTPOW2_56;
			self.chessBoard.updateZobristKey(ROOK_BLACK, 58);
			self.chessBoard.updateZobristKey(ROOK_BLACK, 56);
		} else {

			self.chessBoard.chessboard[KING_BLACK] = self.chessBoard.chessboard[KING_BLACK] | POW2_61 & NOTPOW2_59;
			self.chessBoard.updateZobristKey(KING_BLACK, 61);
			self.chessBoard.updateZobristKey(KING_BLACK, 59);
			self.chessBoard.chessboard[ROOK_BLACK] = self.chessBoard.chessboard[ROOK_BLACK] | POW2_60 & NOTPOW2_63;
			self.chessBoard.updateZobristKey(ROOK_BLACK, 60);
			self.chessBoard.updateZobristKey(ROOK_BLACK, 63);
		}
	}
}

func ( self *GenMoves ) unPerformCastle(side uint, typee uint64) {
	if side == WHITE {
		if typee & KING_SIDE_CASTLE_MOVE_MASK != 0 {

			self.chessBoard.chessboard[KING_WHITE] = (self.chessBoard.chessboard[KING_WHITE] | POW2_3) & NOTPOW2_1;
			self.chessBoard.chessboard[ROOK_WHITE] = (self.chessBoard.chessboard[ROOK_WHITE] | POW2_0) & NOTPOW2_2;
		} else {
			self.chessBoard.chessboard[KING_WHITE] = (self.chessBoard.chessboard[KING_WHITE] | POW2_3) & NOTPOW2_5;
			self.chessBoard.chessboard[ROOK_WHITE] = (self.chessBoard.chessboard[ROOK_WHITE] | POW2_7) & NOTPOW2_4;
		}
	} else {
		if typee & KING_SIDE_CASTLE_MOVE_MASK != 0 {
			self.chessBoard.chessboard[KING_BLACK] = (self.chessBoard.chessboard[KING_BLACK] | POW2_59) & NOTPOW2_57;
			self.chessBoard.chessboard[ROOK_BLACK] = (self.chessBoard.chessboard[ROOK_BLACK] | POW2_56) & NOTPOW2_58;
		} else {
			self.chessBoard.chessboard[KING_BLACK] = (self.chessBoard.chessboard[KING_BLACK] | POW2_59) & NOTPOW2_61;
			self.chessBoard.chessboard[ROOK_BLACK] = (self.chessBoard.chessboard[ROOK_BLACK] | POW2_63) & NOTPOW2_60;
		}
	}
}

func ( self *GenMoves ) tryAllCastle(side uint, allpieces uint64) {

	if side == WHITE {
		if POW2_3 & self.chessBoard.chessboard[KING_WHITE] != 0 && allpieces & 0x6 == 0 && self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK != 0  && self.chessBoard.chessboard[ROOK_WHITE] & POW2_0 != 0  && 0 == self.isAttacked(WHITE, 1, allpieces) && 0 == self.isAttacked(WHITE, 2, allpieces) && 0 == self.isAttacked(WHITE, 3, allpieces) {
			self.pushmove(KING_SIDE_CASTLE_MOVE_MASK, 0xffffffff, 0xffffffff, WHITE, NO_PROMOTION, 0xffffffff);
		}
		if POW2_3 & self.chessBoard.chessboard[KING_WHITE] != 0 && allpieces & 0x70 == 0 && self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK != 0 && self.chessBoard.chessboard[ROOK_WHITE] & POW2_7 != 0  && 0 == self.isAttacked(WHITE, 3, allpieces) && 0 == self.isAttacked(WHITE, 4, allpieces) && 0 == self.isAttacked(WHITE, 5, allpieces) {
			self.pushmove(QUEEN_SIDE_CASTLE_MOVE_MASK, 0xffffffff, 0xffffffff, WHITE, NO_PROMOTION, 0xffffffff);
		}
	} else {
		if POW2_59 & self.chessBoard.chessboard[KING_BLACK] != 0 && self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK != 0 && 0 == (allpieces & 0x600000000000000) && self.chessBoard.chessboard[ROOK_BLACK] & POW2_56 != 0 && 0 == self.isAttacked(BLACK, 57, allpieces) && 0 == self.isAttacked(BLACK, 58, allpieces) && 0 == self.isAttacked(BLACK, 59, allpieces) {
			self.pushmove(KING_SIDE_CASTLE_MOVE_MASK, 0xffffffff, 0xffffffff, BLACK, NO_PROMOTION, 0xffffffff);
		}
		if POW2_59 & self.chessBoard.chessboard[KING_BLACK] != 0&& self.chessBoard.chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK != 0 && 0 == (allpieces & 0x7000000000000000) && self.chessBoard.chessboard[ROOK_BLACK] & POW2_63 != 0 && 0 == self.isAttacked(BLACK, 59, allpieces) && 0 == self.isAttacked(BLACK, 60, allpieces) && 0 == self.isAttacked(BLACK, 61, allpieces) {
			self.pushmove(QUEEN_SIDE_CASTLE_MOVE_MASK, 0xffffffff, 0xffffffff, BLACK, NO_PROMOTION, 0xffffffff);
		}
	}
}

func ( self *GenMoves ) pushmove(typee uint64, from uint, to uint, side uint, promotionPiece int, pieceFrom uint) bool {

	var piece_captured = SQUARE_FREE;
	var res = false;
	if ((typee & 0x3) != ENPASSANT_MOVE_MASK) && 0 == (typee & 0xc) {

		piece_captured = self.chessBoard.getPieceAt(side ^ 1, POW2[to])

		if piece_captured == KING_BLACK + (side ^ 1) {
			res = true;
		}
	} else if typee & 0xc == 0 {
		//no castle
		piece_captured = side ^ 1;
	}
	//if (typee & 0xc) != 0 && (self.forceCheck || self.perftMode)
	{
		//no castle
		if side == WHITE && self.inCheck(WHITE, typee, from, to, pieceFrom, piece_captured, uint(promotionPiece)) != 0 {
			return false;
		}
		if side == BLACK && self.inCheck(BLACK, typee, from, to, pieceFrom, piece_captured, uint(promotionPiece)) != 0 {
			return false;
		}
	}
	var mos*_Tmove;

	mos = &self.gen_list[self.listId].moveList[self.getListSize()];
	self.gen_list[self.listId].size = self.gen_list[self.listId].size + 1;
	mos.typee = self.chessBoard.chessboard[RIGHT_CASTLE_IDX] | typee;
	mos.side = side;
	mos.capturedPiece = piece_captured;
	if typee & 0x3 != 0 {
		mos.from = from;
		mos.to = to;
		mos.pieceFrom = pieceFrom;
		mos.promotionPiece = promotionPiece;
		if self.perftMode == false {
			if res == true {
				mos.score = _INFINITE;
			} else {

				//mos.score = self.killerHeuristic[from][to];
				if PIECES_VALUE[piece_captured] >= PIECES_VALUE[pieceFrom] {
					mos.score = mos.score + (PIECES_VALUE[piece_captured] - PIECES_VALUE[pieceFrom]) * 2;
				} else {
					mos.score = mos.score + PIECES_VALUE[piece_captured];
				}
			}
		}
	} else if typee & 0xc != 0 {
		//castle

		mos.score = 100;
	}
	mos.used = false;

	return res;
}

func ( self *GenMoves ) getMove(i uint) *_Tmove {
	return &self.gen_list[self.listId].moveList[i];
}

func ( self *GenMoves ) inCheck1(side uint) uint64 {
	return self.isAttacked(side, BITScanForward(self.chessBoard.chessboard[KING_BLACK + side]), self.chessBoard.getBitmap(BLACK) | self.chessBoard.getBitmap(WHITE));
}

func ( self *GenMoves ) checkJumpPawn(side uint, xx uint64, xallpieces uint64) {
	var x = xx;
	x &= TABJUMPPAWN;
	if side != 0 {
		x = (((x << 8) & xallpieces) << 8) & xallpieces;
	} else {
		x = (((x >> 8) & xallpieces) >> 8) & xallpieces;
	};
	for ; x != 0; {
		var o = BITScanForward(x);
		var rr int;
		if side != 0 {
			rr = -16;
		} else {
			rr = 16;
		}
		var xx = uint(int(o) + rr);
		self.pushmove(STANDARD_MOVE_MASK, xx, o, side, NO_PROMOTION, side);
		x = reset_lsb(x);
	}
}

func ( self *GenMoves ) performRankFileCaptureAndShiftCount(position uint, enemies uint64, allpieces uint64) uint {

	var rankFile uint64 = self.chessBoard.bitboard.getRankFile(position, allpieces);
	rankFile = (rankFile & enemies) | (rankFile & ^allpieces);
	return bitCount(rankFile)
}

func ( self *GenMoves ) getAttackers(side uint, exitOnFirst bool, position uint, allpieces uint64) uint64 {

	//knight
	var attackers = KNIGHT_MASK[position] & self.chessBoard.chessboard[KNIGHT_BLACK + (side ^ 1)];
	if exitOnFirst == true && attackers != 0 {
		return 1
	};
	//king
	attackers |= NEAR_MASK1[position] & self.chessBoard.chessboard[KING_BLACK + (side ^ 1)];
	if exitOnFirst == true && attackers != 0 {
		return 1
	};
	//pawn
	attackers |= PAWN_FORK_MASK[side][position] & self.chessBoard.chessboard[PAWN_BLACK + (side ^ 1)];
	if exitOnFirst == true && attackers != 0 {
		return 1
	};
	//bishop queen
	var enemies = self.chessBoard.chessboard[BISHOP_BLACK + (side ^ 1)] | self.chessBoard.chessboard[QUEEN_BLACK + (side ^ 1)];
	var nuovo = self.chessBoard.bitboard.getDiagonalAntiDiagonal(position, allpieces) & enemies;
	for ; nuovo != 0; {
		var bound = BITScanForward(nuovo);
		attackers |= POW2[bound];
		if exitOnFirst == true && attackers != 0 {
			return 1
		};
		nuovo = reset_lsb(nuovo);
	}
	enemies = self.chessBoard.chessboard[ROOK_BLACK + (side ^ 1)] | self.chessBoard.chessboard[QUEEN_BLACK + (side ^ 1)];
	nuovo = self.chessBoard.bitboard.getRankFile(position, allpieces) & enemies;
	for ; nuovo != 0; {
		var bound = BITScanForward(nuovo);
		attackers |= POW2[bound];
		if exitOnFirst == true && attackers != 0 {
			return 1
		};
		nuovo = reset_lsb(nuovo);
	}
	return attackers;
}

func ( self *GenMoves ) perft(side uint, depthx uint) uint64 {

	if depthx == 0 {
		return 1
	}

	var n_perft uint64 = 0

	self.incListId()

	var friends = self.chessBoard.getBitmap(side)
	var enemies = self.chessBoard.getBitmap(side ^ 1)
	var b = self.generateCaptures(side, enemies, friends)
//self.display();
	assert(b == false, "assert captured king");
	self.generateMoves(side, friends | enemies);
	var listcount = self.getListSize();

	if listcount == 0 {
		self.decListId();
		return 0;
	}
	for ii := 0; ii < listcount; ii++ {
		move := self.getMove(uint(ii));
		var keyold = self.chessBoard.chessboard[ZOBRISTKEY_IDX];
		self.makemove(move, false, false);
		n_perft += self.perft(side ^ 1, depthx - 1);
		self.takeback(move, keyold, false);
	}
	self.decListId()

	return n_perft
}