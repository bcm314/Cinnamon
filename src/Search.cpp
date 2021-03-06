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


#include "Search.h"
#include "SearchManager.h"
#include "db/bitbase/kpk.h"
#include "namespaces/board.h"

bool volatile Search::runningThread;
high_resolution_clock::time_point Search::startTime;
using namespace _bitbase;
void Search::run() {
    if (getRunning()) {
        if (searchMovesVector.size())
            aspirationWindow<true>(mainDepth, valWindow);
        else
            aspirationWindow<false>(mainDepth, valWindow);
    }
}

template<bool searchMoves>
void Search::aspirationWindow(const int depth, const int valWin) {
    valWindow = valWin;
    init();

    if (depth == 1) {
        valWindow = search<searchMoves>(depth, -_INFINITE - 1, _INFINITE + 1);
    } else {
        int tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW);

        if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
            if (tmp <= valWindow - VAL_WINDOW) {
                tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW * 2, valWindow + VAL_WINDOW);
            } else {
                tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 2);
            }

            if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                if (tmp <= valWindow - VAL_WINDOW) {
                    tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW * 4, valWindow + VAL_WINDOW);
                } else {
                    tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 4);
                }

                if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                    tmp = search<searchMoves>(mainDepth, -_INFINITE - 1, _INFINITE + 1);
                }
            }
        }
        if (getRunning()) {
            valWindow = tmp;
        }
    }
}

Search::Search() : ponder(false), nullSearch(false) {
#ifdef DEBUG_MODE
    lazyEvalCuts = cumulativeMovesCount = totGen = 0;
#endif

}

void Search::clone(const Search *s) {
    memcpy(chessboard, s->chessboard, sizeof(_Tchessboard));
}

#ifndef JS_MODE
void Search::printDtmGtb() {
    int side = getSide();
    u64 friends = side == WHITE ? getBitmap<WHITE>() : getBitmap<BLACK>();
    u64 enemies = side == BLACK ? getBitmap<WHITE>() : getBitmap<BLACK>();
    display();
    cout << "current: ";
    SearchManager::getGtb()->getDtm(side, true, chessboard, 100);
    fflush(stdout);
    incListId();
    generateCaptures(side, enemies, friends);
    generateMoves(side, friends | enemies);
    _Tmove *move;
    u64 oldKey = 0;

    for (int i = 0; i < getListSize(); i++) {
        move = &gen_list[listId].moveList[i];
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, false);
            continue;
        };

        cout << endl << decodeBoardinv(move->type, move->from, getSide())
            << decodeBoardinv(move->type, move->to, getSide()) << " ";

        auto res = -SearchManager::getGtb()->getDtm(side ^ 1, true, chessboard, 100);

        if (res != -INT_MAX) {
            cout << " res: " << res;
        }

        takeback(move, oldKey, false);

    }

    cout << endl;

    decListId();

}
#endif

void Search::setNullMove(bool b) {
    nullSearch = !b;
}

void Search::startClock() {
    startTime = std::chrono::high_resolution_clock::now();
}

void Search::setMainPly(int m) {
    mainDepth = m;
}

int Search::checkTime() {
    if (getRunning() == 2) {
        return 2;
    }
    if (ponder) {
        return 1;
    }
    auto t_current = std::chrono::high_resolution_clock::now();
    return Time::diffTime(t_current, startTime) >= maxTimeMillsec ? 0 : 1;
}

Search::~Search() {
    join();
}

template<int side>
int Search::quiescence(int alpha, int beta, const char promotionPiece, int N_PIECE, int depth) {
    if (!getRunning()) {
        return 0;
    }

    const u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];
    int score = getScore(zobristKeyR, side, N_PIECE, alpha, beta, false);
    if (score >= beta) {
        return beta;
    }
    ///************* hash ****************
    char hashf = Hash::hashfALPHA;

    pair<int, _TcheckHash> hashAlwaysItem = checkHash(Hash::HASH_ALWAYS, true, alpha, beta, depth, zobristKeyR);
    if (hashAlwaysItem.first != INT_MAX) {
        return hashAlwaysItem.first;
    };

    pair<int, _TcheckHash> hashGreaterItem = checkHash(Hash::HASH_GREATER, true, alpha, beta, depth, zobristKeyR);
    if (hashGreaterItem.first != INT_MAX) {
        return hashGreaterItem.first;
    };

///********** end hash ***************
/**************Delta Pruning ****************/
    char fprune = 0;
    int fscore;
    if ((fscore = score + (promotionPiece == NO_PROMOTION ? VALUEQUEEN : 2 * VALUEQUEEN)) < alpha) {
        fprune = 1;
    }
/************ end Delta Pruning *************/
    if (score > alpha) {
        alpha = score;
    }

    incListId();

    u64 friends = getBitmap<side>();
    u64 enemies = getBitmap<side ^ 1>();
    if (generateCaptures<side>(enemies, friends)) {
        decListId();

        return _INFINITE - (mainDepth + depth);
    }
    if (!getListSize()) {
        --listId;
        return score;
    }
    _Tmove *move;
    _Tmove *best = &gen_list[listId].moveList[0];
    const u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    if (hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS].dataS.flags & 0x3) {
        sortFromHash(listId, hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS]);
    } else if (hashGreaterItem.second.phasheType[Hash::HASH_GREATER].dataS.flags & 0x3) {
        sortFromHash(listId, hashGreaterItem.second.phasheType[Hash::HASH_GREATER]);
    }

    while ((move = getNextMove(&gen_list[listId]))) {
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, false);
            continue;
        }
/**************Delta Pruning ****************/
        if (fprune && ((move->type & 0x3) != PROMOTION_MOVE_MASK) &&
            fscore + PIECES_VALUE[move->capturedPiece] <= alpha) {
            INC(nCutFp);
            takeback(move, oldKey, false);
            continue;
        }
/************ end Delta Pruning *************/
        int val = -quiescence<side ^ 1>(-beta, -alpha, move->promotionPiece, N_PIECE - 1, depth - 1);
        score = max(score, val);
        takeback(move, oldKey, false);
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                if (getRunning()) {
                    Hash::_ThashData data(score, depth, move->from, move->to, 0, Hash::hashfBETA);
                    hash->recordHash(zobristKeyR, data);
                }
                return beta;
            }
            best = move;
            alpha = score;
            hashf = Hash::hashfEXACT;
        }
    }
    if (getRunning()) {
        Hash::_ThashData data(score, depth, best->from, best->to, 0, hashf);
        hash->recordHash(zobristKeyR, data);
    }
    decListId();

    return score;
}

void Search::setPonder(bool r) {
    ponder = r;
}

void Search::setRunning(int r) {
    GenMoves::setRunning(r);
    if (!r) {
        maxTimeMillsec = 0;
    }
}

int Search::getRunning() {
    if (!runningThread)return 0;
    return GenMoves::getRunning();

}

void Search::setMaxTimeMillsec(int n) {
    maxTimeMillsec = n;
}

int Search::getMaxTimeMillsec() {
    return maxTimeMillsec;
}

void Search::sortFromHash(const int listId, const Hash::_ThashData &phashe) {
    for (int r = 0; r < gen_list[listId].size; r++) {
        _Tmove *mos = &gen_list[listId].moveList[r];
        ASSERT_RANGE(phashe.dataS.from, 0, 63);
        ASSERT_RANGE(phashe.dataS.to, 0, 63);
        if (phashe.dataS.from == mos->from && phashe.dataS.to == mos->to) {
            mos->score = _INFINITE / 2;
            return;
        }
    }
}

bool Search::checkInsufficientMaterial(int nPieces) {
    //regexp: KN?B*KB*
    switch (nPieces) {
        case 2 :
            //KK
            return true;
        case 3:
            //KBK
            if (chessboard[BISHOP_BLACK] || chessboard[BISHOP_WHITE])return true;
            //KNK
            if (chessboard[KNIGHT_BLACK] || chessboard[KNIGHT_WHITE])return true;
            break;
        case 4 :
            //KBKB
            if (chessboard[BISHOP_BLACK] && chessboard[BISHOP_WHITE])return true;
            //KNKN
            if (chessboard[KNIGHT_BLACK] && chessboard[KNIGHT_WHITE])return true;
            //KBKN
            if (chessboard[BISHOP_BLACK] && chessboard[KNIGHT_WHITE])return true;
            if (chessboard[BISHOP_WHITE] && chessboard[KNIGHT_BLACK])return true;
            //KNNK
            if (bitCount(chessboard[KNIGHT_BLACK]) == 2)return true;
            if (bitCount(chessboard[KNIGHT_WHITE]) == 2)return true;
            break;
        default:
            return false;
    }
    return false;
}

bool Search::checkDraw(u64 key) {
    int o = 0;
    int count = 0;
    for (int i = repetitionMapCount - 1; i >= 0; i--) {
        if (repetitionMap[i] == 0) {
            return false;
        }

        //fifty-move rule
        if (++count > 100) {
            return true;
        }

        //Threefold repetition
        if (repetitionMap[i] == key && ++o > 2) {
            return true;
        }
    }
    return false;
}

void Search::setMainParam(const int depth) {
    memset(&pvLine, 0, sizeof(_TpvLine));
    mainDepth = depth;
}

template<bool searchMoves>
int Search::search(const int depth, const int alpha, const int beta) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    incListId();
    const int side = getSide();
    const u64 friends = (side == WHITE) ? getBitmap<WHITE>() : getBitmap<BLACK>();
    const u64 enemies = (side == WHITE) ? getBitmap<BLACK>() : getBitmap<WHITE>();
    generateCaptures(side, enemies, friends);
    generateMoves(side, friends | enemies);
    int n_root_moves = getListSize();
    decListId();
    return getSide() ? search<WHITE, searchMoves>(depth,
                                                  alpha,
                                                  beta,
                                                  &pvLine,
                                                  bitCount(getBitmap<WHITE>() | getBitmap<BLACK>()),
                                                  &mainMateIn,
                                                  n_root_moves)
                     : search<BLACK, searchMoves>(depth, alpha, beta, &pvLine,
                                                  bitCount(getBitmap<WHITE>() | getBitmap<BLACK>()), &mainMateIn,
                                                  n_root_moves);
}

string Search::probeRootTB() {
    const auto tot = bitCount(getBitmap<WHITE>() | getBitmap<BLACK>());
    const int side = getSide();
    string best = "";
#ifndef JS_MODE
    if (SearchManager::getGtb() && SearchManager::getGtb()->isInstalledPieces(tot)) {
        u64 friends = side == WHITE ? getBitmap<WHITE>() : getBitmap<BLACK>();
        u64 enemies = side == BLACK ? getBitmap<WHITE>() : getBitmap<BLACK>();
        //display();

        incListId();
        generateCaptures(side, enemies, friends);
        generateMoves(side, friends | enemies);

        u64 oldKey = 0;

        int bestRes = INT_MAX;
        _Tmove *bestMove = nullptr;
        _Tmove *drawMove = nullptr;
        for (int i = 0; i < getListSize(); i++) {
            _Tmove *move = &gen_list[listId].moveList[i];
            if (!makemove(move, false, true)) {
                takeback(move, oldKey, false);
                continue;
            }

            auto dtm = SearchManager::getGtb()->getDtm(side ^ 1, false, chessboard, 100);

            if (dtm != INT_MAX) {

                if (dtm == GTB_DRAW) {
                    drawMove = move;
                } else if (bestRes == INT_MAX) {
                    bestRes = dtm;
                    bestMove = move;
                }
                else if (dtm < 0 && bestRes < 0 && dtm > bestRes) {
                    bestRes = dtm;
                    bestMove = move;
                }
                else if (dtm < 0 && bestRes > 0) {
                    bestRes = dtm;
                    bestMove = move;
                }
                else if (dtm > 0 && bestRes > 0 && dtm < bestRes) {

                    bestMove = move;
                }
            }
            takeback(move, oldKey, false);
        }
        if (bestRes > 0 && drawMove) {
            bestMove = drawMove;
        }

        ASSERT(bestMove != nullptr)
        best = string(decodeBoardinv(bestMove->type, bestMove->from, getSide())) +
            string(decodeBoardinv(bestMove->type, bestMove->to, getSide()));
        if (bestMove->promotionPiece != -1)best += "q";
        decListId();

        return best;
    }

#endif
    //kpk -> try draw
    if (tot == 3 && chessboard[side] == 0 && chessboard[side ^ 1]) {

        u64 friends = side == WHITE ? getBitmap<WHITE>() : getBitmap<BLACK>();
        u64 enemies = side == BLACK ? getBitmap<WHITE>() : getBitmap<BLACK>();

        incListId();
        generateCaptures(side, enemies, friends);
        generateMoves(side, friends | enemies);

        u64 oldKey = 0;

        _Tmove *bestMove = nullptr;

        for (int i = 0; i < getListSize(); i++) {
            if (bestMove)break;
            _Tmove *move = &gen_list[listId].moveList[i];
            if (!makemove(move, true, true)) {
                takeback(move, oldKey, false);
                continue;
            }

            const int kw = BITScanForward(chessboard[KING_WHITE]);
            const int kb = BITScanForward(chessboard[KING_BLACK]);
            const int
                pawnPos =
                side == WHITE ? BITScanForward(chessboard[PAWN_BLACK]) : BITScanForward(chessboard[PAWN_WHITE]);


            auto draw =
                side == BLACK ? isDraw<WHITE>(side ^ 1, kw, kb, pawnPos) : isDraw<BLACK>(side ^ 1, kw, kb, pawnPos);

            if (draw) {
                bestMove = move;
            }

            takeback(move, oldKey, false);
        }

        if (bestMove)
            best = string(decodeBoardinv(bestMove->type, bestMove->from, getSide())) +
                string(decodeBoardinv(bestMove->type, bestMove->to, getSide()));

        decListId();

        return best;
    }
    return "";
}

template<bool checkMoves>
bool Search::checkSearchMoves(_Tmove *move) {
    if (!checkMoves)return true;
    int m = move->to | (move->from << 8);
    if (std::find(searchMovesVector.begin(), searchMovesVector.end(), m) != searchMovesVector.end()) {
        return true;
    }
    return false;
}

template<int side, bool checkMoves>
int Search::search(int depth, int alpha, int beta, _TpvLine *pline, int N_PIECE, int *mateIn, int n_root_moves) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    INC(cumulativeMovesCount);

    ASSERT_RANGE(side, 0, 1);
    if (!getRunning()) {
        return 0;
    }

    *mateIn = INT_MAX;
    int score = -_INFINITE;

    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
#ifdef DEBUG_MODE
    double betaEfficiencyCount = 0.0;
#endif
    ASSERT(chessboard[KING_BLACK]);
    ASSERT(chessboard[KING_WHITE]);
    ASSERT(chessboard[KING_BLACK + side]);
    int extension = 0;
    const int is_incheck_side = inCheck<side>();
    if (!is_incheck_side && depth != mainDepth) {
        if (checkInsufficientMaterial(N_PIECE) || checkDraw(chessboard[ZOBRISTKEY_IDX])) {
            if (inCheck<side ^ 1>()) {
                return _INFINITE - (mainDepth - depth + 1);
            }
            return -lazyEval<side>() * 2;
        }
    }
    if (is_incheck_side) {
        extension++;
    }
    depth += extension;
    if (depth == 0) {
        return quiescence<side>(alpha, beta, -1, N_PIECE, 0);
    }

    //************* hash ****************
    u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];

    pair<int, _TcheckHash> hashGreaterItem = checkHash(Hash::HASH_GREATER, false, alpha, beta, depth, zobristKeyR);
    if (hashGreaterItem.first != INT_MAX) {
        return hashGreaterItem.first;
    };

    pair<int, _TcheckHash> hashAlwaysItem = checkHash(Hash::HASH_ALWAYS, false, alpha, beta, depth, zobristKeyR);
    if (hashAlwaysItem.first != INT_MAX) {
        return hashAlwaysItem.first;
    };
    ///********** end hash ***************

    if (!(numMoves % 8192)) {
        setRunning(checkTime());
    }
    ++numMoves;
    _TpvLine line;
    line.cmove = 0;

    // ********* null move ***********
    if (!nullSearch && /*!pv_node &&*/  !is_incheck_side) {
        int n_depth = (n_root_moves > 17 || depth > 3) ? 1 : 3;
        if (n_depth == 3) {
            const u64 pieces = getPiecesNoKing<side>();
            if (pieces != chessboard[PAWN_BLACK + side] || bitCount(pieces) > 9)
                n_depth = 1;
        }
        if (depth > n_depth) {
            nullSearch = true;
            const int R = NULL_DEPTH + depth / NULL_DIVISOR;
            const int nullScore =
                (depth - R - 1 > 0) ?
                -search<side ^ 1, checkMoves>(depth - R - 1, -beta, -beta + 1, &line, N_PIECE, mateIn, n_root_moves)
                                    :
                -quiescence<side ^ 1>(-beta, -beta + 1, -1, N_PIECE, 0);
            nullSearch = false;
            if (nullScore >= beta) {
                INC(nNullMoveCut);
                return nullScore;
            }
        }
    }

    ///******* null move end ********

    /**************Futility Pruning****************/
    /**************Futility Pruning razor at pre-pre-frontier*****/
    bool futilPrune = false;
    int futilScore = 0;
    if (depth <= 3 && !is_incheck_side) {
        int matBalance = lazyEval<side>();
        if ((futilScore = matBalance + FUTIL_MARGIN) <= alpha) {
            if (depth == 3 && (matBalance + RAZOR_MARGIN) <= alpha && getNpiecesNoPawnNoKing<side ^ 1>() > 3) {
                INC(nCutRazor);
                depth--;
            } else
                ///**************Futility Pruning at pre-frontier*****
            if (depth == 2 && (futilScore = matBalance + EXT_FUTILY_MARGIN) <= alpha) {
                futilPrune = true;
                score = futilScore;
            } else
                ///**************Futility Pruning at frontier*****
            if (depth == 1) {
                futilPrune = true;
                score = futilScore;
            }
        }
    }
    /************ end Futility Pruning*************/
    incListId();
    ASSERT_RANGE(KING_BLACK + side, 0, 11);
    ASSERT_RANGE(KING_BLACK + (side ^ 1), 0, 11);
    const u64 friends = getBitmap<side>();
    const u64 enemies = getBitmap<side ^ 1>();
    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        score = _INFINITE - (mainDepth - depth + 1);
        return score;
    }
    generateMoves<side>(friends | enemies);
    int listcount = getListSize();
    if (!listcount) {
        --listId;
        if (is_incheck_side) {
            return -_INFINITE + (mainDepth - depth + 1);
        } else {
            return -lazyEval<side>() * 2;
        }
    }
    ASSERT(gen_list[listId].size > 0);
    _Tmove *best = &gen_list[listId].moveList[0];
    if (hashGreaterItem.second.phasheType[Hash::HASH_GREATER].dataS.flags & 0x3) {
        sortFromHash(listId, hashGreaterItem.second.phasheType[Hash::HASH_GREATER]);
    } else if (hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS].dataS.flags & 0x3) {
        sortFromHash(listId, hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS]);
    }
    INC(totGen);
    _Tmove *move;
    bool checkInCheck = false;
    int countMove = 0;
    char hashf = Hash::hashfALPHA;
    while ((move = getNextMove(&gen_list[listId]))) {
        if (!checkSearchMoves<checkMoves>(move) && depth == mainDepth)continue;
        countMove++;
        INC(betaEfficiencyCount);
        if (!makemove(move, true, checkInCheck)) {
            takeback(move, oldKey, true);
            continue;
        }
        checkInCheck = true;
        if (futilPrune && ((move->type & 0x3) != PROMOTION_MOVE_MASK) &&
            futilScore + PIECES_VALUE[move->capturedPiece] <= alpha && !inCheck<side>()) {
            INC(nCutFp);
            takeback(move, oldKey, true);
            continue;
        }
        //Late Move Reduction
        int val = INT_MAX;
        if (countMove > 4 && !is_incheck_side && depth >= 3 && move->capturedPiece == SQUARE_FREE &&
            move->promotionPiece == NO_PROMOTION) {
            currentPly++;
            val = -search<side ^ 1, checkMoves>(depth - 2, -(alpha + 1), -alpha, &line, N_PIECE, mateIn, n_root_moves);
            ASSERT(val != INT_MAX);
            currentPly--;
        }
        if (val > alpha) {
            const int doMws = (score > -_INFINITE + MAX_PLY);
            const int lwb = max(alpha, score);
            const int upb = (doMws ? (lwb + 1) : beta);
            currentPly++;
            val = -search<side ^ 1, checkMoves>(depth - 1, -upb, -lwb, &line,
                                                move->capturedPiece == SQUARE_FREE ? N_PIECE : N_PIECE - 1,
                                                mateIn, n_root_moves);
            ASSERT(val != INT_MAX);
            currentPly--;
            if (doMws && (lwb < val) && (val < beta)) {
                currentPly++;
                val = -search<side ^ 1, checkMoves>(depth - 1, -beta, -val + 1,
                                                    &line, move->capturedPiece == SQUARE_FREE ? N_PIECE : N_PIECE - 1,
                                                    mateIn, n_root_moves);
                currentPly--;
            }
        }
        score = max(score, val);
        takeback(move, oldKey, true);
        move->score = score;
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                ASSERT(move->score == score);
                INC(nCutAB);
                ADD(betaEfficiency, betaEfficiencyCount / (double) listcount * 100.0);
                if (getRunning()) {
                    Hash::_ThashData data(score, depth - extension, move->from, move->to, 0, Hash::hashfBETA);
                    hash->recordHash(zobristKeyR, data);
                }
                if (depth < 31)
                    setHistoryHeuristic(move->from, move->to, 1 << depth);
                else
                    setHistoryHeuristic(move->from, move->to, 0x40000000);
                return score;
            }
            alpha = score;
            hashf = Hash::hashfEXACT;
            best = move;
            move->score = score;    //used in it
            updatePv(pline, &line, move);
        }
    }
    if (getRunning()) {
        Hash::_ThashData data(score, depth - extension, best->from, best->to, 0, hashf);
        hash->recordHash(zobristKeyR, data);
    }
    decListId();
    return score;
}

void Search::updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move) {
    ASSERT(line->cmove < MAX_PLY - 1);
    memcpy(&(pline->argmove[0]), move, sizeof(_Tmove));
    memcpy(pline->argmove + 1, line->argmove, line->cmove * sizeof(_Tmove));
    ASSERT(line->cmove >= 0);
    pline->cmove = line->cmove + 1;
}

_Tchessboard &Search::getChessboard() {
    return chessboard;
}

void Search::setChessboard(_Tchessboard &b) {
    memcpy(chessboard, b, sizeof(chessboard));
}

u64 Search::getZobristKey() {
    return chessboard[ZOBRISTKEY_IDX];
}

int Search::getMateIn() {
    return mainMateIn;
}


void Search::unsetSearchMoves() {
    searchMovesVector.clear();
}

void Search::setSearchMoves(vector<int> &s) {
    searchMovesVector = s;
}

bool Search::setParameter(String param, int value) {
#if defined(CLOP) || defined(DEBUG_MODE)
    param.toUpper();
    bool res = true;
    if (param == "FUTIL_MARGIN") {
        FUTIL_MARGIN = value;
    } else if (param == "EXT_FUTILY_MARGIN") {
        EXT_FUTILY_MARGIN = value;
    } else if (param == "RAZOR_MARGIN") {
        RAZOR_MARGIN = value;
    } else if (param == "ATTACK_KING") {
        ATTACK_KING = value;
    } else if (param == "BACKWARD_PAWN") {
        BACKWARD_PAWN = value;
    } else if (param == "BISHOP_ON_QUEEN") {
        BISHOP_ON_QUEEN = value;
    } else if (param == "BONUS2BISHOP") {
        BONUS2BISHOP = value;
    } else if (param == "CONNECTED_ROOKS") {
        CONNECTED_ROOKS = value;
    } else if (param == "DOUBLED_ISOLATED_PAWNS") {
        DOUBLED_ISOLATED_PAWNS = value;
    } else if (param == "DOUBLED_PAWNS") {
        DOUBLED_PAWNS = value;
    } else if (param == "END_OPENING") {
        END_OPENING = value;
    } else if (param == "ENEMY_NEAR_KING") {
        ENEMY_NEAR_KING = value;
    } else if (param == "FRIEND_NEAR_KING") {
        FRIEND_NEAR_KING = value;
    } else if (param == "HALF_OPEN_FILE_Q") {
        HALF_OPEN_FILE_Q = value;
    } else if (param == "OPEN_FILE") {
        OPEN_FILE = value;
    } else if (param == "OPEN_FILE_Q") {
        OPEN_FILE_Q = value;
    } else if (param == "PAWN_IN_7TH") {
        PAWN_IN_7TH = value;
    } else if (param == "PAWN_CENTER") {
        PAWN_CENTER = value;
    } else if (param == "PAWN_IN_8TH") {
        PAWN_IN_8TH = value;
    } else if (param == "PAWN_ISOLATED") {
        PAWN_ISOLATED = value;
    } else if (param == "PAWN_NEAR_KING") {
        PAWN_NEAR_KING = value;
    } else if (param == "PAWN_BLOCKED") {
        PAWN_BLOCKED = value;
    } else if (param == "ROOK_7TH_RANK") {
        ROOK_7TH_RANK = value;
    } else if (param == "ROOK_BLOCKED") {
        ROOK_BLOCKED = value;
    } else if (param == "ROOK_TRAPPED") {
        ROOK_TRAPPED = value;
    } else if (param == "UNDEVELOPED_KNIGHT") {
        UNDEVELOPED_KNIGHT = value;
    } else if (param == "UNDEVELOPED_BISHOP") {
        UNDEVELOPED_BISHOP = value;
    } else if (param == "VAL_WINDOW") {
        VAL_WINDOW = value;
    } else if (param == "UNPROTECTED_PAWNS") {
        UNPROTECTED_PAWNS = value;
    } else {
        res = false;
    }
    return res;
#else
    cout << param << " " << value << endl;
    return false;
#endif
}



