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

#include "Eval.h"

using namespace _eval;
u64 *Eval::evalHash;

#ifdef BENCH_MODE
Time Eval::evalTime;
Time Eval::bishopTime;
Time Eval::pawnTime;
Time Eval::kingTime;
Time Eval::rookTime;
Time Eval::knightTime;
Time Eval::queenTime;
#endif

Eval::Eval() {
    if (evalHash == nullptr)
        evalHash = (u64 *) calloc(hashSize, sizeof(u64));
}

Eval::~Eval() {
    free(evalHash);
    evalHash = nullptr;
}

template<int side>
void Eval::openFile() {
    structureEval.openFile = 0;
    structureEval.semiOpenFile[side] = 0;

    for (u64 side_rooks = chessboard[ROOK_BLACK + side]; side_rooks; RESET_LSB(side_rooks)) {
        const int o = BITScanForward(side_rooks);
        if (!(FILE_[o] & (chessboard[WHITE] | chessboard[BLACK])))
            structureEval.openFile |= FILE_[o];
        else if (FILE_[o] & chessboard[side ^ 1])
            structureEval.semiOpenFile[side] |= FILE_[o];
    }
}

/**
 * evaluate pawns for color at phase
 * 1. if no pawns returns -NO_PAWNS
 * 3. if other side has all pawns substracts ENEMIES_ALL_PAWNS
 * 4. add ATTACK_KING * number of attacking pawn to other king
 * 5. space - in OPEN phase PAWN_CENTER * CENTER_MASK
 * 6. // pinned - in END phase substracts 20 * each pinned pawn
 * 7. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING * each pawn near to king and substracts ENEMY_NEAR_KING * each enemy pawn near to king
 * 8. pawn in 8th - if pawn is in 7' add PAWN_7H. If pawn can go forward add PAWN_IN_8TH for each pawn
 * 9. unprotected - no friends pawn protect it
 * 10. blocked - pawn can't go on
 * 11. isolated - there aren't friend pawns on the two sides - subtracts PAWN_ISOLATED for each pawn
 * 12. doubled - there aren't friend pawns on the two sides - subtracts DOUBLED_PAWNS for each pawn. If it is isolated too substracts DOUBLED_ISOLATED_PAWNS
 * 13. backward - if there isn't friend pawns on sides or on sides in 1 rank below subtracts BACKWARD_PAWN
 * 14. passed - if there isn't friend pawns forward and forward on sides until 8' rank add PAWN_PASSED[side][pos]

 */
template<int side>
pair<short, short>  Eval::evaluatePawn() {
    INC(evaluationCount[side]);
    const u64 ped_friends = chessboard[side];
    if (!ped_friends) {
        ADD(SCORE_DEBUG[EG].NO_PAWNS[side], -NO_PAWNS);
        ADD(SCORE_DEBUG[MG].NO_PAWNS[side], -NO_PAWNS);
        return pair<short, short>(-NO_PAWNS, -NO_PAWNS);
    }

    int result_eg_mg = 0;
    int result_eg = 0;
    int result_mg = 0;
    constexpr int xside = side ^1;
    if (bitCount(chessboard[xside]) == 8) {
        result_eg_mg -= ENEMIES_PAWNS_ALL;
        ADD(SCORE_DEBUG[EG].ENEMIES_PAWNS_ALL[side], -ENEMIES_PAWNS_ALL);
        ADD(SCORE_DEBUG[MG].ENEMIES_PAWNS_ALL[side], -ENEMIES_PAWNS_ALL);
    }

// 5. space
    const auto x1 = bitCount(ped_friends & CENTER_MASK);
    result_mg += PAWN_CENTER[MG] * x1;
    result_eg += PAWN_CENTER[EG] * x1;
    ADD(SCORE_DEBUG[MG].PAWN_CENTER[side], PAWN_CENTER[MG] * x1);
    ADD(SCORE_DEBUG[EG].PAWN_CENTER[side], PAWN_CENTER[EG] * x1);

    // 6. pinned
//    if (phase == END) {
//        result -= 20 * bitCount(structureEval.pinned[side] & ped_friends);
//    }


    // 7.
    const auto x3 = bitCount(NEAR_MASK2[structureEval.posKing[side]] & ped_friends);
    structureEval.kingSecurity[MG][side] += FRIEND_NEAR_KING[MG] * x3;
    structureEval.kingSecurity[EG][side] += FRIEND_NEAR_KING[EG] * x3;

    const auto x2 = bitCount(NEAR_MASK2[structureEval.posKing[xside]] & ped_friends);
    structureEval.kingSecurity[MG][side] -= ENEMY_NEAR_KING[MG] * x2;
    structureEval.kingSecurity[EG][side] -= ENEMY_NEAR_KING[EG] * x2;

    // 8.  pawn in 8th
    const u64 pawnsIn7 = PAWNS_7_2[side] & ped_friends;
    result_eg += PAWN_IN_7TH * bitCount(pawnsIn7);
    ADD(SCORE_DEBUG[EG].PAWN_7H[side], PAWN_IN_7TH * bitCount(pawnsIn7));

    const u64 pawnsIn8 = (shiftForward<side, 8>(pawnsIn7) & (~structureEval.allPieces)) |
        (structureEval.allPiecesSide[xside] & (shiftForward<side, 7>(pawnsIn7) | shiftForward<side, 9>(pawnsIn7)));

    result_eg += PAWN_IN_8TH * bitCount(pawnsIn8); //try to decrease PAWN_IN_8TH
    ADD(SCORE_DEBUG[EG].PAWN_IN_8TH[side], PAWN_IN_8TH * (bitCount(pawnsIn8)));

    for (u64 p = ped_friends; p; RESET_LSB(p)) {
        bool isolated = false;
        const int o = BITScanForward(p);
        const u64 pos = POW2[o];

        // 4. attack king
        if (structureEval.posKingBit[xside] & PAWN_FORK_MASK[side][o]) {
            structureEval.kingAttackers[xside] |= pos;
            result_mg += ATTACK_KING[MG];
            result_eg += ATTACK_KING[EG];
        }

        /// blocked
        result_eg_mg -= (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
            (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? PAWN_BLOCKED : 0;
        ADD(SCORE_DEBUG[EG].PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
            (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? -PAWN_BLOCKED : 0);
        ADD(SCORE_DEBUG[MG].PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
            (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? -PAWN_BLOCKED : 0);
        /// unprotected
        if (!(ped_friends & PAWN_PROTECTED_MASK[side][o])) {
            result_eg_mg -= UNPROTECTED_PAWNS;
            ADD(SCORE_DEBUG[EG].UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS);
            ADD(SCORE_DEBUG[MG].UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS);
        }
        /// isolated
        if (!(ped_friends & PAWN_ISOLATED_MASK[o])) {
            result_mg -= PAWN_ISOLATED[MG];
            result_eg -= PAWN_ISOLATED[EG];
            ADD(SCORE_DEBUG[EG].PAWN_ISOLATED[side], -PAWN_ISOLATED[EG]);
            ADD(SCORE_DEBUG[MG].PAWN_ISOLATED[side], -PAWN_ISOLATED[MG]);
            isolated = true;
        }
        /// doubled
        if (NOTPOW2[o] & FILE_[o] & ped_friends) {
            result_eg_mg -= DOUBLED_PAWNS;
            ADD(SCORE_DEBUG[EG].DOUBLED_PAWNS[side], -DOUBLED_PAWNS);
            ADD(SCORE_DEBUG[MG].DOUBLED_PAWNS[side], -DOUBLED_PAWNS);
            /// doubled and isolated
            if (isolated) {
                ADD(SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS);
                ADD(SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS);
                result_eg_mg -= DOUBLED_ISOLATED_PAWNS;
            }
        }
        /// backward
        if (!(ped_friends & PAWN_BACKWARD_MASK[side][o])) {
            ADD(SCORE_DEBUG[EG].BACKWARD_PAWN[side], -BACKWARD_PAWN[EG]);
            ADD(SCORE_DEBUG[MG].BACKWARD_PAWN[side], -BACKWARD_PAWN[MG]);
            result_eg -= BACKWARD_PAWN[EG];
            result_mg -= BACKWARD_PAWN[MG];
        }
        /// passed
        if (!(chessboard[xside] & PAWN_PASSED_MASK[side][o])) {
            ADD(SCORE_DEBUG[EG].PAWN_PASSED[side], PAWN_PASSED[EG][side][o]);
            ADD(SCORE_DEBUG[MG].PAWN_PASSED[side], PAWN_PASSED[MG][side][o]);
            result_eg += PAWN_PASSED[EG][side][o];
            result_mg += PAWN_PASSED[MG][side][o];
        }
    }
    result_eg += result_eg_mg;
    result_mg += result_eg_mg;
    return pair<short, short>(result_mg, result_eg);
}

/**
 * evaluate bishop for color at phase
 * 1. if no bishops returns 0
 * 2. if two bishops add BONUS2BISHOP
 * 3 *king security* - in OPEN phase substracts at kingSecurity ENEMY_NEAR_KING for each bishop close to enmey king
 * 4. undevelop - substracts UNDEVELOPED_BISHOP for each undeveloped bishop
 * 5. mobility add MOB_BISHOP[phase][???]
 * 6. if only one bishop and pawns on same square color substracts n_pawns * BISHOP_PAWN_ON_SAME_COLOR
 * pinned ?
 * 7. outposts
 * 8. bishop on big diagonal
 */
template<int side>
pair<short, short>  Eval::evaluateBishop(const u64 enemies) {
    INC(evaluationCount[side]);
    u64 bishop = chessboard[BISHOP_BLACK + side];

    // 1.
    if (!bishop) return pair<short, short>(0, 0);

    int result_eg = 0;
    int result_mg = 0;
    int result_eg_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    const int nBishop = bitCount(bishop);

    // 2.
    if (nBishop == 1) {
        result_eg_mg -=
            BISHOP_PAWN_ON_SAME_COLOR * bitCount(chessboard[side] & ChessBoard::colors(BITScanForward(bishop)));
    } else {
        // 2.
        result_eg += BONUS2BISHOP;
        ADD(SCORE_DEBUG[EG].BONUS2BISHOP[side], BONUS2BISHOP);
    }

    // 3. *king security*
    const auto near = bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & bishop);
    structureEval.kingSecurity[MG][side] -= ENEMY_NEAR_KING[MG] * near;
    structureEval.kingSecurity[EG][side] -= ENEMY_NEAR_KING[EG] * near;
    ADD(SCORE_DEBUG[EG].KING_SECURITY_BISHOP[side], -ENEMY_NEAR_KING[EG] * near);
    ADD(SCORE_DEBUG[MG].KING_SECURITY_BISHOP[side], -ENEMY_NEAR_KING[MG] * near);

    // 4. undevelop
    result_mg -= UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop);
    ADD(SCORE_DEBUG[MG].UNDEVELOPED_BISHOP[side], UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop));

    for (; bishop; RESET_LSB(bishop)) {
        const int o = BITScanForward(bishop);

        // 5. mobility
        u64 captured = getDiagCapture(o, structureEval.allPieces, enemies);
        ASSERT(bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces) <
            (int) (sizeof(MOB_BISHOP) / sizeof(int)));

        if (captured & structureEval.posKingBit[side ^ 1]) {
            structureEval.kingAttackers[side ^ 1] |= POW2[o];
        }
        const auto x1 = bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces);
        result_eg += MOB_BISHOP[EG][x1];
        result_mg += MOB_BISHOP[MG][x1];
        ADD(SCORE_DEBUG[EG].MOB_BISHOP[side], MOB_BISHOP[EG][x1]);
        ADD(SCORE_DEBUG[MG].MOB_BISHOP[side], MOB_BISHOP[MG][x1]);

        // 6.
        if ((BIG_DIAGONAL & structureEval.allPieces) == POW2[o]) {
            ADD(SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[side], OPEN_FILE);
            result_eg += OPEN_FILE;
        }
        if ((BIG_ANTIDIAGONAL & structureEval.allPieces) == POW2[o]) {
            ADD(SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[side], OPEN_FILE);
            result_eg += OPEN_FILE;
        }

        // 7. outposts
        const auto p = BISHOP_OUTPOST[side][o];
        constexpr int xside = side ^1;
        //enemy pawn doesn't attack bishop
        if (p && !(PAWN_FORK_MASK[side ^ 1][o] & chessboard[side ^ 1])) {
            //friend paws defends bishop
            if (PAWN_FORK_MASK[side ^ 1][o] & chessboard[side]) {
                result_eg_mg += p;
                if (!(chessboard[KNIGHT_BLACK + xside]) &&
                    !(chessboard[BISHOP_BLACK + xside] & ChessBoard::colors(o))) {
                    result_eg_mg += p;
                }
            }
        }
    }
    result_eg += result_eg_mg;
    result_mg += result_eg_mg;
    return pair<short, short>(result_mg, result_eg);
}

/**
 * evaluate queen for color at phase
 * 1. // pinned
 * 2. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING for each queen near to king and substracts ENEMY_NEAR_KING for each queen near to enemy king
 * 3. mobility - MOB_QUEEN[phase][position]
 * 4. half open file - if there is a enemy pawn on same file add HALF_OPEN_FILE_Q
 * 5. open file - if there is any pieces on same file add OPEN_FILE_Q
 * 6. 5. bishop on queen - if there is a bishop on same diagonal add BISHOP_ON_QUEEN
 */
template<int side>
pair<short, short>  Eval::evaluateQueen(const u64 enemies) {
    INC(evaluationCount[side]);
    u64 queen = chessboard[QUEEN_BLACK + side];
    int result_eg_mg = 0;//20 * bitCount(structureEval.pinned[side] & queen);
    int result_eg = 0;
    int result_mg = 0;

    // 2. *king security*
    const auto x1 = bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen);
    structureEval.kingSecurity[MG][side] += FRIEND_NEAR_KING[MG] * x1;
    structureEval.kingSecurity[EG][side] += FRIEND_NEAR_KING[EG] * x1;
    ADD(SCORE_DEBUG[EG].KING_SECURITY_QUEEN[side], FRIEND_NEAR_KING[EG] * x1);
    ADD(SCORE_DEBUG[MG].KING_SECURITY_QUEEN[side], FRIEND_NEAR_KING[MG] * x1);

    const auto x2 = bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & queen);
    structureEval.kingSecurity[EG][side] -= ENEMY_NEAR_KING[EG] * x2;
    structureEval.kingSecurity[MG][side] -= ENEMY_NEAR_KING[MG] * x2;

    ADD(SCORE_DEBUG[EG].KING_SECURITY_QUEEN[side ^ 1], -ENEMY_NEAR_KING[EG] * x2);
    ADD(SCORE_DEBUG[MG].KING_SECURITY_QUEEN[side ^ 1], -ENEMY_NEAR_KING[MG] * x2);

    for (; queen; RESET_LSB(queen)) {
        const int o = BITScanForward(queen);
        // 3. mobility
        const u64 x = getMobilityQueen(o, enemies, structureEval.allPieces);
        result_eg += MOB_QUEEN[EG][bitCount(x)];
        result_mg += MOB_QUEEN[MG][bitCount(x)];
        ADD(SCORE_DEBUG[EG].MOB_QUEEN[side], MOB_QUEEN[EG][bitCount(x)]);
        ADD(SCORE_DEBUG[MG].MOB_QUEEN[side], MOB_QUEEN[MG][bitCount(x)]);

        if (x & structureEval.posKingBit[side ^ 1])
            structureEval.kingAttackers[side ^ 1] |= POW2[o];
        // 4. half open file
        if ((chessboard[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG[EG].HALF_OPEN_FILE_Q[side], HALF_OPEN_FILE_Q);
            ADD(SCORE_DEBUG[MG].HALF_OPEN_FILE_Q[side], HALF_OPEN_FILE_Q);
            result_eg_mg += HALF_OPEN_FILE_Q;
        }

        // 5. open file
        if ((FILE_[o] & structureEval.allPieces) == POW2[o]) {
            ADD(SCORE_DEBUG[EG].OPEN_FILE_Q[side], OPEN_FILE_Q);
            ADD(SCORE_DEBUG[MG].OPEN_FILE_Q[side], OPEN_FILE_Q);
            result_eg_mg += OPEN_FILE_Q;
        }

        // 6. bishop on queen
        if (DIAGONAL_ANTIDIAGONAL[o] & chessboard[BISHOP_BLACK + side]) {
            ADD(SCORE_DEBUG[EG].BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN);
            ADD(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN);
            result_eg_mg += BISHOP_ON_QUEEN;
        }
    }
    result_eg += result_eg_mg;
    result_mg += result_eg_mg;
    return pair<short, short>(result_mg, result_eg);
}

/**
 * evaluate knight for color at phase
 * 1. // pinned
 * 2. undevelop - substracts UNDEVELOPED_KNIGHT for each undeveloped knight
 * 3. trapped TODO
 * 4. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING for each knight near to king and substracts ENEMY_NEAR_KING for each knight near to enemy king
 * 5. mobility
 * 6. outposts
*/

template<int side>
pair<short, short>  Eval::evaluateKnight(const u64 enemiesPawns, const u64 notMyBits) {
    INC(evaluationCount[side]);
    u64 knight = chessboard[KNIGHT_BLACK + side];
    if (!knight) return pair<short, short>(0, 0);
    constexpr int xside = side ^1;
    // 1. pinned
    int result_eg_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    int result_eg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    int result_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);

    // 2. undevelop
    result_mg -= bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT;
    ADD(SCORE_DEBUG[MG].UNDEVELOPED_KNIGHT[side],
        bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT);

    // 3. trapped
    if (side == WHITE) {
        if ((A7bit & knight) && (B7bit & enemiesPawns) && (C6A6bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
        if ((H7bit & knight) && (G7bit & enemiesPawns) && (F6H6bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
        if ((A8bit & knight) && (A7C7bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
        if ((H8bit & knight) && (H7G7bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
    } else {
        if ((A2bit & knight) && (B2bit & enemiesPawns) && (C3A3bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
        if ((H2bit & knight) && (G2bit & enemiesPawns) && (F3H3bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
        if ((A1bit & knight) && (A2C2bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
        if ((H1bit & knight) && (H2G2bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_eg_mg -= KNIGHT_TRAPPED;
        }
    }

    // 4. king security
    const auto x1 = bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight);
    structureEval.kingSecurity[EG][side] += FRIEND_NEAR_KING[EG] * x1;
    structureEval.kingSecurity[MG][side] += FRIEND_NEAR_KING[MG] * x1;
    ADD(SCORE_DEBUG[EG].KING_SECURITY_KNIGHT[side], FRIEND_NEAR_KING[EG] * x1);
    ADD(SCORE_DEBUG[MG].KING_SECURITY_KNIGHT[side], FRIEND_NEAR_KING[MG] * x1);
    const auto x2 = bitCount(NEAR_MASK2[structureEval.posKing[xside]] & knight);
    structureEval.kingSecurity[EG][side] -= ENEMY_NEAR_KING[EG] * x2;
    structureEval.kingSecurity[MG][side] -= ENEMY_NEAR_KING[MG] * x2;

    ADD(SCORE_DEBUG[EG].KING_SECURITY_KNIGHT[xside], -ENEMY_NEAR_KING[EG] * x2);
    ADD(SCORE_DEBUG[MG].KING_SECURITY_KNIGHT[xside], -ENEMY_NEAR_KING[MG] * x2);

    for (; knight; RESET_LSB(knight)) {
        const int pos = BITScanForward(knight);

        // 5. mobility
        ASSERT(bitCount(notMyBits & KNIGHT_MASK[pos]) < (int) (sizeof(MOB_KNIGHT) / sizeof(int)));
        const u64 mob = notMyBits & KNIGHT_MASK[pos];
        result_eg_mg += MOB_KNIGHT[bitCount(mob)];
        if (mob & structureEval.posKingBit[xside]) structureEval.kingAttackers[xside] |= POW2[pos];
        ADD(SCORE_DEBUG[EG].MOB_KNIGHT[side], MOB_KNIGHT[bitCount(mob)]);
        ADD(SCORE_DEBUG[MG].MOB_KNIGHT[side], MOB_KNIGHT[bitCount(mob)]);

        // 6. outposts
        const auto p = KNIGHT_OUTPOST[side][pos];

        //enemy pawn doesn't attack knight
        if (p && !(PAWN_FORK_MASK[xside][pos] & chessboard[xside])) {
            //friend paws defends knight
            if (PAWN_FORK_MASK[xside][pos] & chessboard[side]) {
                result_eg_mg += p;
                if (!(chessboard[KNIGHT_BLACK + xside]) &&
                    !(chessboard[BISHOP_BLACK + xside] & ChessBoard::colors(pos))) {
                    result_eg_mg += p;
                }
            }
        }
    }
    result_eg += result_eg_mg;
    result_mg += result_eg_mg;
    return pair<short, short>(result_mg, result_eg);
}


/**
 * evaluate rook for color at phase
 * 1. if no rooks returns 0
 * 2. // pinned
 * 3. in middle if in 7th - add ROOK_7TH_RANK for each rook in 7th
 * 4. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING for each rook near to king and substracts ENEMY_NEAR_KING for each rook near to enemy king
 * 5. add OPEN_FILE/HALF_OPEN_FILE if the rook is on open/semiopen file
 * 6. trapped
 * 7. 2 linked towers
 * 8. Penalise if Rook is Blocked Horizontally
*/
template<int side>
pair<short, short>  Eval::evaluateRook(const u64 king, const u64 enemies, const u64 friends) {
    INC(evaluationCount[side]);

    u64 rook = chessboard[ROOK_BLACK + side];
    if (!rook) return pair<short, short>(0, 0);

    const int nRooks = bitCount(rook);
    // 2.
    int result_eg_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    int result_eg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    int result_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    constexpr int xside = side ^1;
    // 3. in 7th
    result_mg += ROOK_7TH_RANK * bitCount(rook & RANK_1_7[side]);
    ADD(SCORE_DEBUG[MG].ROOK_7TH_RANK[side], ROOK_7TH_RANK * bitCount(rook & RANK_1_7[side]));

    // 4. king security
    auto x = bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook);
    structureEval.kingSecurity[EG][side] += FRIEND_NEAR_KING[EG] * x;
    structureEval.kingSecurity[MG][side] += FRIEND_NEAR_KING[MG] * x;
    ADD(SCORE_DEBUG[EG].KING_SECURITY_ROOK[side], FRIEND_NEAR_KING[EG] * x);
    ADD(SCORE_DEBUG[MG].KING_SECURITY_ROOK[side], FRIEND_NEAR_KING[MG] * x);
    x = bitCount(NEAR_MASK2[structureEval.posKing[xside]] & rook);
    structureEval.kingSecurity[EG][side] -= ENEMY_NEAR_KING[EG] * x;
    structureEval.kingSecurity[MG][side] -= ENEMY_NEAR_KING[MG] * x;
    ADD(SCORE_DEBUG[EG].KING_SECURITY_ROOK[xside], -ENEMY_NEAR_KING[EG] * x);
    ADD(SCORE_DEBUG[MG].KING_SECURITY_ROOK[xside], -ENEMY_NEAR_KING[MG] * x);

    // .6
    if (((F1G1bit[side] & king) && (H1H2G1bit[side] & rook)) || ((C1B1bit[side] & king) && (A1A2B1bit[side] & rook))) {
        ADD(SCORE_DEBUG[EG].ROOK_TRAPPED[side], -ROOK_TRAPPED);
        ADD(SCORE_DEBUG[MG].ROOK_TRAPPED[side], -ROOK_TRAPPED);
        result_eg_mg -= ROOK_TRAPPED;
    }

    // .7
    if (nRooks == 2) {
        const int firstRook = BITScanForward(rook);
        const int secondRook = BITScanReverse(rook);
        if ((!(LINK_ROOKS[firstRook][secondRook] & structureEval.allPieces))) {
            ADD(SCORE_DEBUG[EG].CONNECTED_ROOKS[side], CONNECTED_ROOKS);
            ADD(SCORE_DEBUG[MG].CONNECTED_ROOKS[side], CONNECTED_ROOKS);
            result_eg_mg += CONNECTED_ROOKS;
        }
    }

    for (; rook; RESET_LSB(rook)) {
        const int o = BITScanForward(rook);
        //mobility
        const u64 mob = getMobilityRook(o, enemies, friends);
        if (mob & structureEval.posKingBit[side ^ 1]) structureEval.kingAttackers[side ^ 1] |= POW2[o];

        ASSERT(bitCount(mob) < (int) (sizeof(MOB_ROOK[EG]) / sizeof(int)));
        result_eg_mg += MOB_ROOK[EG][bitCount(mob)];
        result_eg_mg += MOB_ROOK[MG][bitCount(mob)];
        ADD(SCORE_DEBUG[EG].MOB_ROOK[side], MOB_ROOK[EG][bitCount(mob)]);
        ADD(SCORE_DEBUG[MG].MOB_ROOK[side], MOB_ROOK[MG][bitCount(mob)]);

        // .8 Penalise if Rook is Blocked Horizontally
        if ((RANK_BOUND[o] & structureEval.allPieces) == RANK_BOUND[o]) {
            ADD(SCORE_DEBUG[EG].ROOK_BLOCKED[side], -ROOK_BLOCKED);
            result_eg -= ROOK_BLOCKED;
        }

        // .5
        if (!(chessboard[side] & FILE_[o])) {
            ADD(SCORE_DEBUG[EG].ROOK_OPEN_FILE[side], OPEN_FILE);
            ADD(SCORE_DEBUG[MG].ROOK_OPEN_FILE[side], OPEN_FILE);
            result_eg_mg += OPEN_FILE;
        }
        if (!(chessboard[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG[EG].ROOK_OPEN_FILE[side], OPEN_FILE);
            ADD(SCORE_DEBUG[MG].ROOK_OPEN_FILE[side], OPEN_FILE);
            result_eg_mg += OPEN_FILE;
        }
    }
    result_eg += result_eg_mg;
    result_mg += result_eg_mg;
    return pair<short, short>(result_mg, result_eg);
}

pair<short, short>  Eval::evaluateKing(int side, u64 squares) {
    ASSERT(evaluationCount[side] == 5);
    int result_eg_mg = 0;
    int result_eg = 0;
    int result_mg = 0;
    const uchar pos_king = structureEval.posKing[side];
    ADD(SCORE_DEBUG[EG].DISTANCE_KING[side], DISTANCE_KING_ENDING[pos_king]);
    result_eg = DISTANCE_KING_ENDING[pos_king];
    ADD(SCORE_DEBUG[MG].DISTANCE_KING[side], DISTANCE_KING_OPENING[pos_king]);
    result_mg = DISTANCE_KING_OPENING[pos_king];
    const u64 POW2_king = POW2[pos_king];
    //mobility
    ASSERT(bitCount(squares & NEAR_MASK1[pos_king]) < (int) (sizeof(MOB_KING[EG]) / sizeof(int)));
    const auto x = bitCount(squares & NEAR_MASK1[pos_king]);
    result_eg += MOB_KING[EG][x];
    result_mg += MOB_KING[MG][x];
    ADD(SCORE_DEBUG[EG].MOB_KING[side], MOB_KING[EG][x]);
    ADD(SCORE_DEBUG[MG].MOB_KING[side], MOB_KING[MG][x]);

    if ((structureEval.openFile & POW2_king) || (structureEval.semiOpenFile[side ^ 1] & POW2_king)) {
        ADD(SCORE_DEBUG[EG].END_OPENING_KING[side], -END_OPENING);
        result_eg -= END_OPENING;
        if (bitCount(RANK[pos_king]) < 4) {
            ADD(SCORE_DEBUG[EG].END_OPENING_KING[side], -END_OPENING);
            result_eg -= END_OPENING;
        }
    }

    ASSERT(pos_king < 64);
    if (!(NEAR_MASK1[pos_king] & chessboard[side])) {
        ADD(SCORE_DEBUG[EG].PAWN_NEAR_KING[side], -PAWN_NEAR_KING);
        ADD(SCORE_DEBUG[MG].PAWN_NEAR_KING[side], -PAWN_NEAR_KING);
        result_eg_mg -= PAWN_NEAR_KING;
    }

    result_eg += result_eg_mg + structureEval.kingSecurity[EG][side];
    result_mg += result_eg_mg + structureEval.kingSecurity[MG][side];

    return pair<short, short>(result_mg, result_eg);
}

void Eval::storeHashValue(const u64 key, const short value) {
    evalHash[key % hashSize] = (key & keyMask) | (value & valueMask);
    ASSERT(value == getHashValue(key));
}

short Eval::getHashValue(const u64 key) const {
    const u64 kv = evalHash[key % hashSize];
    if ((kv & keyMask) == (key & keyMask))
        return (short) (kv & valueMask);

    return noHashValue;
}

short Eval::getScore(const u64 key, const int side, const int N_PIECE, const int alpha, const int beta,
                     const bool trace) {
    BENCH(evalTime.start());
    const short hashValue = getHashValue(key);
    if (hashValue != noHashValue) {
        BENCH(evalTime.stop());
        return hashValue;
    }
    const int lazyscore_white = lazyEvalSide<WHITE>();
    const int lazyscore_black = lazyEvalSide<BLACK>();
    const int lazyscore = side ? -(lazyscore_black - lazyscore_white) : lazyscore_black - lazyscore_white;

    if (lazyscore > (beta + FUTIL_MARGIN) || lazyscore < (alpha - FUTIL_MARGIN)) {
        INC(lazyEvalCuts);
        BENCH(evalTime.stop());
        return lazyscore;
    }

#ifdef DEBUG_MODE
    evaluationCount[WHITE] = evaluationCount[BLACK] = 0;
    memset(&SCORE_DEBUG, 0, sizeof(_TSCORE_DEBUG));
#endif

    memset(structureEval.kingSecurity, 0, sizeof(structureEval.kingSecurity));

    structureEval.allPiecesNoPawns[BLACK] = getBitmapNoPawns<BLACK>();
    structureEval.allPiecesNoPawns[WHITE] = getBitmapNoPawns<WHITE>();
    structureEval.allPiecesSide[BLACK] = structureEval.allPiecesNoPawns[BLACK] | chessboard[PAWN_BLACK];
    structureEval.allPiecesSide[WHITE] = structureEval.allPiecesNoPawns[WHITE] | chessboard[PAWN_WHITE];
    structureEval.allPieces = structureEval.allPiecesSide[BLACK] | structureEval.allPiecesSide[WHITE];
    structureEval.posKing[BLACK] = (uchar) BITScanForward(chessboard[KING_BLACK]);
    structureEval.posKing[WHITE] = (uchar) BITScanForward(chessboard[KING_WHITE]);
    structureEval.kingAttackers[WHITE] = structureEval.kingAttackers[BLACK] = 0;
//    if (phase == END) {
//
//        structureEval.pinned[BLACK] = getPinned<BLACK>(structureEval.allPieces, structureEval.allPiecesSide[BLACK],
//                                                       structureEval.posKing[BLACK]);
//
//        structureEval.pinned[WHITE] = getPinned<WHITE>(structureEval.allPieces, structureEval.allPiecesSide[WHITE],
//                                                       structureEval.posKing[WHITE]);
//    } else {
//        structureEval.pinned[BLACK] = structureEval.pinned[WHITE] = 0;
//    }
    openFile<WHITE>();
    openFile<BLACK>();

    const short bonus_attack_king_blackEG = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[WHITE])];
    const short bonus_attack_king_whiteEG = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[BLACK])];

    _Tresult tresult[2];
    getScores(tresult);

    ASSERT(getMobilityCastle(WHITE, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[MG]) / sizeof(int)));
    ASSERT(getMobilityCastle(BLACK, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[MG]) / sizeof(int)));
    const short mobW = getMobilityCastle(WHITE, structureEval.allPieces);
    const int mobWhite_mg = MOB_CASTLE[MG][mobW];
    const int mobWhite_eg = MOB_CASTLE[EG][mobW];
    const short mobB = getMobilityCastle(BLACK, structureEval.allPieces);
    const int mobBlack_mg = MOB_CASTLE[MG][mobB];
    const int mobBlack_eg = MOB_CASTLE[EG][mobB];
    const int attack_king_white_mg = ATTACK_KING[MG] * bitCount(structureEval.kingAttackers[BLACK]);
    const int attack_king_white_eg = ATTACK_KING[EG] * bitCount(structureEval.kingAttackers[BLACK]);
    const int attack_king_black_mg = ATTACK_KING[MG] * bitCount(structureEval.kingAttackers[WHITE]);
    const int attack_king_black_eg = ATTACK_KING[EG] * bitCount(structureEval.kingAttackers[WHITE]);

    const int result_mg = (mobBlack_mg + attack_king_black_mg + lazyscore_black + tresult[MG].pawns[BLACK] +
        tresult[MG].knights[BLACK] + tresult[MG].bishop[BLACK] + tresult[MG].rooks[BLACK] + tresult[MG].queens[BLACK] +
        tresult[MG].kings[BLACK]) -
        (mobWhite_mg + attack_king_white_mg + lazyscore_white + tresult[MG].pawns[WHITE] +
            tresult[MG].knights[WHITE] + tresult[MG].bishop[WHITE] + tresult[MG].rooks[WHITE]
            + tresult[MG].queens[WHITE] +
            tresult[MG].kings[WHITE]);

    const int result_eg =
        (mobBlack_eg + attack_king_black_eg + bonus_attack_king_blackEG + lazyscore_black + tresult[EG].pawns[BLACK] +
            tresult[EG].knights[BLACK] + tresult[EG].bishop[BLACK] + tresult[EG].rooks[BLACK]
            + tresult[EG].queens[BLACK] + tresult[EG].kings[BLACK]) -
            (mobWhite_eg + attack_king_white_eg + bonus_attack_king_whiteEG + lazyscore_white + tresult[EG].pawns[WHITE]
                + tresult[EG].knights[WHITE] + tresult[EG].bishop[WHITE] + tresult[EG].rooks[WHITE]
                + tresult[EG].queens[WHITE] + tresult[EG].kings[WHITE]);

//    const double percMG = ((double) (lazyscore_white + lazyscore_black) / MAX_VALUE_TAPERED);
//    const short score = (percMG * (double) result_mg) + ((1.0 - percMG) * (double) result_eg); //round
//    ASSERT(percMG >= 0 && percMG <= 1);

    constexpr int PawnPhase = 0;
    constexpr int KnightPhase = 1;
    constexpr int BishopPhase = 1;
    constexpr int RookPhase = 2;
    constexpr int QueenPhase = 4;
    constexpr int TotalPhase = PawnPhase * 16 + KnightPhase * 4 + BishopPhase * 4 + RookPhase * 4 + QueenPhase * 2;

    int phase = TotalPhase;

    phase -= bitCount(chessboard[PAWN_BLACK]) * PawnPhase;
    phase -= bitCount(chessboard[PAWN_WHITE]) * PawnPhase;
    phase -= bitCount(chessboard[KNIGHT_WHITE]) * KnightPhase;
    phase -= bitCount(chessboard[KNIGHT_BLACK]) * KnightPhase;
    phase -= bitCount(chessboard[BISHOP_BLACK]) * BishopPhase;
    phase -= bitCount(chessboard[BISHOP_WHITE]) * BishopPhase;
    phase -= bitCount(chessboard[QUEEN_BLACK]) * QueenPhase;
    phase -= bitCount(chessboard[QUEEN_WHITE]) * QueenPhase;


    phase = (phase * 256 + (TotalPhase / 2)) / TotalPhase;
    const int score = ((result_mg * (256 - phase)) + (result_eg * phase)) / 256;

    const auto finalScore = side ? -(score - 5) : score + 5;

#ifdef DEBUG_MODE
    if (trace) {
//        const string HEADER = "\n";//|\t\t\t\t\tWHITE\t\tBLACK\t\t  WHITE\t\tBLACK\t\t  WHITE\t\tBLACK\n";

        cout << "|OPEN FILE: ";
        if (!structureEval.openFile)cout << "none";
        else
            for (int i = 0; i < 8; i++) if (POW2[i] & structureEval.openFile)cout << (char) (65 + i) << " ";
        cout << endl;

        cout << "|VALUES:";
        cout << "\tPAWN: " << (double) _board::VALUEPAWN / 100.0;
        cout << " ROOK: " << (double) _board::VALUEROOK / 100.0;
        cout << " BISHOP: " << (double) _board::VALUEBISHOP / 100.0;
        cout << " KNIGHT: " << (double) _board::VALUEKNIGHT / 100.0;
        cout << " QUEEN: " << (double) _board::VALUEQUEEN / 100.0 << endl << endl;

//        cout << HEADER;
        cout << "|Material:         " << setw(10) << (double) (lazyscore_white - lazyscore_black) / 100.0 << setw(15)
            << (double) (lazyscore_white) / 100.0 << setw(10) << (double) (lazyscore_black) / 100.0 << endl;
        cout << "|Mobility:         " << setw(10) << (double) (mobWhite_mg - mobBlack_mg) / 100.0 << setw(15)
            << (double) (mobWhite_mg) / 100.0 << setw(10) << (double) (mobBlack_mg) / 100.0 << endl;
        cout << "|Bonus attack king (EG):" << setw(5)
            << (double) (bonus_attack_king_whiteEG - bonus_attack_king_blackEG) / 100.0 << setw(15)
            << (double) (bonus_attack_king_whiteEG) / 100.0 << setw(10) << (double) (bonus_attack_king_blackEG) / 100.0
            << endl;
        cout << endl;
        cout << "|\t\tEval term\t|\t   Total\t\t|\t   White\t\t|\t   Black\t\t|\n"
            "|\t\t\t\t\t|\tMG\t\tEG\t\t|\tMG\t\tEG\t\t|\tMG\t\tEG\t\t|\n"
            "|-------------------+-------------------+-------------------+-------------------+" << endl;
        cout << "| PAWN\t\t\t\t|";

        p((tresult[MG].pawns[WHITE] - tresult[MG].pawns[BLACK]), (tresult[EG].pawns[WHITE] - tresult[EG].pawns[BLACK]));

        p(tresult[MG].pawns[WHITE], tresult[MG].pawns[BLACK]);
        p(tresult[EG].pawns[WHITE], tresult[EG].pawns[BLACK]);


        cout << endl << "|attack king\t\t";
        p();
        p(SCORE_DEBUG[MG].ATTACK_KING_PAWN[WHITE], SCORE_DEBUG[MG].ATTACK_KING_PAWN[BLACK]);
        p(SCORE_DEBUG[EG].ATTACK_KING_PAWN[WHITE], SCORE_DEBUG[EG].ATTACK_KING_PAWN[BLACK]);

        cout << endl << "|1 bishop pawn s/c\t";
        p();
        p(SCORE_DEBUG[MG].BISHOP_PAWN_ON_SAME_COLOR[WHITE], SCORE_DEBUG[MG].BISHOP_PAWN_ON_SAME_COLOR[BLACK]);
        p(SCORE_DEBUG[EG].BISHOP_PAWN_ON_SAME_COLOR[WHITE], SCORE_DEBUG[EG].BISHOP_PAWN_ON_SAME_COLOR[BLACK]);


        cout << endl << "|center\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_CENTER[WHITE], SCORE_DEBUG[MG].PAWN_CENTER[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_CENTER[WHITE], SCORE_DEBUG[EG].PAWN_CENTER[BLACK]);

        cout << endl << "|in 7th\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_7H[WHITE], SCORE_DEBUG[MG].PAWN_7H[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_7H[WHITE], SCORE_DEBUG[EG].PAWN_7H[BLACK]);
        cout << endl << "|in 8th\t\t\t\t";

        p();
        p(SCORE_DEBUG[MG].PAWN_IN_8TH[WHITE], SCORE_DEBUG[MG].PAWN_IN_8TH[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_IN_8TH[WHITE], SCORE_DEBUG[EG].PAWN_IN_8TH[BLACK]);

        cout << endl << "|blocked\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_BLOCKED[WHITE], SCORE_DEBUG[MG].PAWN_BLOCKED[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_BLOCKED[WHITE], SCORE_DEBUG[EG].PAWN_BLOCKED[BLACK]);

        cout << endl << "|unprotected\t\t";
        p();
        p(SCORE_DEBUG[MG].UNPROTECTED_PAWNS[WHITE], SCORE_DEBUG[MG].UNPROTECTED_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].UNPROTECTED_PAWNS[WHITE], SCORE_DEBUG[EG].UNPROTECTED_PAWNS[BLACK]);

        cout << endl << "|isolated\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_ISOLATED[WHITE], SCORE_DEBUG[MG].PAWN_ISOLATED[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_ISOLATED[WHITE], SCORE_DEBUG[EG].PAWN_ISOLATED[BLACK]);

        cout << endl << "|double\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].DOUBLED_PAWNS[WHITE], SCORE_DEBUG[MG].DOUBLED_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].DOUBLED_PAWNS[WHITE], SCORE_DEBUG[EG].DOUBLED_PAWNS[BLACK]);

        cout << endl << "|double isolated\t";
        p();
        p(SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[WHITE], SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[WHITE], SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[BLACK]);

        cout << endl << "|backward\t\t\t";
        p();
        p(SCORE_DEBUG[MG].BACKWARD_PAWN[WHITE], SCORE_DEBUG[MG].BACKWARD_PAWN[BLACK]);
        p(SCORE_DEBUG[EG].BACKWARD_PAWN[WHITE], SCORE_DEBUG[EG].BACKWARD_PAWN[BLACK]);

        cout << endl << "|fork\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].FORK_SCORE[WHITE], SCORE_DEBUG[MG].FORK_SCORE[BLACK]);
        p(SCORE_DEBUG[EG].FORK_SCORE[WHITE], SCORE_DEBUG[EG].FORK_SCORE[BLACK]);

        cout << endl << "|passed\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_PASSED[WHITE], SCORE_DEBUG[MG].PAWN_PASSED[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_PASSED[WHITE], SCORE_DEBUG[EG].PAWN_PASSED[BLACK]);

        cout << endl << "|all enemies\t\t";
        p();
        p(SCORE_DEBUG[MG].ENEMIES_PAWNS_ALL[WHITE], SCORE_DEBUG[MG].ENEMIES_PAWNS_ALL[BLACK]);
        p(SCORE_DEBUG[EG].ENEMIES_PAWNS_ALL[WHITE], SCORE_DEBUG[EG].ENEMIES_PAWNS_ALL[BLACK]);

        cout << endl << "|none\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].NO_PAWNS[WHITE], SCORE_DEBUG[MG].NO_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].NO_PAWNS[WHITE], SCORE_DEBUG[EG].NO_PAWNS[BLACK]);

//        cout << HEADER;
        cout << endl << "| KNIGHT\t\t\t\t|";
        p((tresult[MG].knights[WHITE] - tresult[MG].knights[BLACK]),
          (tresult[EG].knights[WHITE] - tresult[EG].knights[BLACK]));

        p(tresult[MG].knights[WHITE], tresult[MG].knights[BLACK]);
        p(tresult[EG].knights[WHITE], tresult[EG].knights[BLACK]);

        cout << endl << "|undevelop\t\t\t";
        p();
        p(SCORE_DEBUG[MG].UNDEVELOPED_KNIGHT[WHITE], SCORE_DEBUG[MG].UNDEVELOPED_KNIGHT[BLACK]);
        p(SCORE_DEBUG[EG].UNDEVELOPED_KNIGHT[WHITE], SCORE_DEBUG[EG].UNDEVELOPED_KNIGHT[BLACK]);

        cout << endl << "|trapped\t\t\t";
        p();
        p(SCORE_DEBUG[MG].KNIGHT_TRAPPED[WHITE], SCORE_DEBUG[MG].KNIGHT_TRAPPED[BLACK]);
        p(SCORE_DEBUG[EG].KNIGHT_TRAPPED[WHITE], SCORE_DEBUG[EG].KNIGHT_TRAPPED[BLACK]);

        cout << endl << "|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_KNIGHT[WHITE], SCORE_DEBUG[MG].MOB_KNIGHT[BLACK]);
        p(SCORE_DEBUG[EG].MOB_KNIGHT[WHITE], SCORE_DEBUG[EG].MOB_KNIGHT[BLACK]);

//        cout << HEADER;
        cout << endl << "| BISHOP\t\t\t\t|";
        p((tresult[MG].bishop[WHITE] - tresult[MG].bishop[BLACK]),
          (tresult[EG].bishop[WHITE] - tresult[EG].bishop[BLACK]));

        p(tresult[MG].bishop[WHITE], tresult[MG].bishop[BLACK]);
        p(tresult[EG].bishop[WHITE], tresult[EG].bishop[BLACK]);

        cout << endl << "|bad\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].BAD_BISHOP[WHITE], SCORE_DEBUG[MG].BAD_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].BAD_BISHOP[WHITE], SCORE_DEBUG[EG].BAD_BISHOP[BLACK]);

        cout << endl << "|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_BISHOP[WHITE], SCORE_DEBUG[MG].MOB_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].MOB_BISHOP[WHITE], SCORE_DEBUG[EG].MOB_BISHOP[BLACK]);

        cout << endl << "|undevelop\t\t\t";
        p();
        p(SCORE_DEBUG[MG].UNDEVELOPED_BISHOP[WHITE], SCORE_DEBUG[MG].UNDEVELOPED_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].UNDEVELOPED_BISHOP[WHITE], SCORE_DEBUG[EG].UNDEVELOPED_BISHOP[BLACK]);

        cout << endl << "|open diag\t\t\t";
        p();
        p(SCORE_DEBUG[MG].OPEN_DIAG_BISHOP[WHITE], SCORE_DEBUG[MG].OPEN_DIAG_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[WHITE], SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[BLACK]);

        cout << endl << "|bonus 2 bishops\t";
        p();
        p(SCORE_DEBUG[MG].BONUS2BISHOP[WHITE], SCORE_DEBUG[MG].BONUS2BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].BONUS2BISHOP[WHITE], SCORE_DEBUG[EG].BONUS2BISHOP[BLACK]);

        cout << endl << "| ROOK\t\t\t\t|";
        p((tresult[MG].rooks[WHITE] - tresult[MG].rooks[BLACK]), (tresult[EG].rooks[WHITE] - tresult[EG].rooks[BLACK]));
        p(tresult[MG].rooks[WHITE], tresult[EG].rooks[BLACK]);
        p(tresult[EG].rooks[BLACK], tresult[EG].rooks[BLACK]);

        cout << endl << "|7th\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_7TH_RANK[WHITE], SCORE_DEBUG[MG].ROOK_7TH_RANK[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_7TH_RANK[WHITE], SCORE_DEBUG[EG].ROOK_7TH_RANK[BLACK]);

        cout << endl << "|trapped\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_TRAPPED[WHITE], SCORE_DEBUG[MG].ROOK_TRAPPED[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_TRAPPED[WHITE], SCORE_DEBUG[EG].ROOK_TRAPPED[BLACK]);

        cout << endl << "|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_ROOK[WHITE], SCORE_DEBUG[MG].MOB_ROOK[BLACK]);
        p(SCORE_DEBUG[EG].MOB_ROOK[WHITE], SCORE_DEBUG[EG].MOB_ROOK[BLACK]);

        cout << endl << "|blocked\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_BLOCKED[WHITE], SCORE_DEBUG[MG].ROOK_BLOCKED[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_BLOCKED[WHITE], SCORE_DEBUG[EG].ROOK_BLOCKED[BLACK]);

        cout << endl << "|open file\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_OPEN_FILE[WHITE], SCORE_DEBUG[MG].ROOK_OPEN_FILE[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_OPEN_FILE[WHITE], SCORE_DEBUG[EG].ROOK_OPEN_FILE[BLACK]);

        cout << endl << "|connected\t\t\t";
        p();
        p(SCORE_DEBUG[MG].CONNECTED_ROOKS[WHITE], SCORE_DEBUG[MG].CONNECTED_ROOKS[BLACK]);
        p(SCORE_DEBUG[EG].CONNECTED_ROOKS[WHITE], SCORE_DEBUG[EG].CONNECTED_ROOKS[BLACK]);

        cout << endl << "| QUEEN\t\t\t\t|";
        p((tresult[MG].queens[WHITE] - tresult[MG].queens[BLACK]),
          (tresult[EG].queens[WHITE] - tresult[EG].queens[BLACK]));
        p(tresult[MG].queens[WHITE], tresult[MG].queens[BLACK]);
        p(tresult[EG].queens[WHITE], tresult[EG].queens[BLACK]);

        cout << endl << "|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_QUEEN[WHITE], SCORE_DEBUG[MG].MOB_QUEEN[BLACK]);
        p(SCORE_DEBUG[MG].MOB_QUEEN[WHITE], SCORE_DEBUG[MG].MOB_QUEEN[BLACK]);

        cout << endl << "|bishop on queen";
        p();
        p(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[WHITE], SCORE_DEBUG[MG].BISHOP_ON_QUEEN[BLACK]);
        p(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[WHITE], SCORE_DEBUG[MG].BISHOP_ON_QUEEN[BLACK]);

        cout << endl << "|King:             " << setw(10)
            << (tresult[MG].kings[WHITE] - tresult[MG].kings[BLACK]) / 100.0
            << setw(15) << (tresult[MG].kings[WHITE]) / 100.0 << setw(10) << (tresult[MG].kings[BLACK]) / 100.0 <<
            setw(10) << (tresult[MG].kings[WHITE] - tresult[MG].kings[BLACK]) / 100.0 << setw(15)
            << (tresult[MG].kings[WHITE]) / 100.0 << setw(10) << (tresult[MG].kings[BLACK]) / 100.0 << endl;
        cout << endl << "|       distance:                 " << setw(10)
            << (SCORE_DEBUG[MG].DISTANCE_KING[WHITE]) / 100.0
            << setw(10) << (SCORE_DEBUG[MG].DISTANCE_KING[BLACK]) / 100.0 <<
            setw(10) << (SCORE_DEBUG[MG].DISTANCE_KING[WHITE]) / 100.0 << setw(10)
            << (SCORE_DEBUG[MG].DISTANCE_KING[BLACK]) / 100.0 << endl;
        cout << endl << "|       open file:                " << setw(10)
            << (SCORE_DEBUG[MG].END_OPENING_KING[WHITE]) / 100.0
            << setw(10) << (SCORE_DEBUG[MG].END_OPENING_KING[BLACK]) / 100.0 <<
            setw(10) << (SCORE_DEBUG[MG].END_OPENING_KING[WHITE]) / 100.0 << setw(10)
            << (SCORE_DEBUG[MG].END_OPENING_KING[BLACK]) / 100.0 << endl;

        cout << endl << "|       pawn near:                " << setw(10)
            << (SCORE_DEBUG[MG].PAWN_NEAR_KING[WHITE]) / 100.0
            << setw(10) << (SCORE_DEBUG[MG].PAWN_NEAR_KING[BLACK]) / 100.0 <<
            setw(10) << (SCORE_DEBUG[MG].PAWN_NEAR_KING[WHITE]) / 100.0 << setw(10)
            << (SCORE_DEBUG[MG].PAWN_NEAR_KING[BLACK]) / 100.0 << endl;
//      cout << "|       mobility:                 " << setw(10) <<  (SCORE_DEBUG[MG].MOB_KING[WHITE]) / 100.0 << setw(10) <<  (SCORE_DEBUG[MG].MOB_KING[BLACK]) / 100.0 << "\n";
        cout << endl;
        cout << endl << "|Total (white)..........   " << (double) (side ? finalScore / 100.0 : -finalScore / 100.0)
            << endl;
    }
#endif
    storeHashValue(key, finalScore);
    BENCH(evalTime.stop());
    return finalScore;
}

