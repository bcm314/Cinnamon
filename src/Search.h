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

#include <sys/timeb.h>
#include <unistd.h>
#include "Hash.h"
#include "Eval.h"
#include "namespaces/def.h"
#include <climits>
#include "threadPool/Thread.h"
#include "db/GTB.h"
#include "db/syzygy/SYZYGY.h"

class Search: public Eval, public Thread<Search> {

public:

    Search();

    Search(const Search *s) { clone(s); }

    void clone(const Search *);

    virtual ~Search();

    void setRunning(int);

    void setPonder(bool);

    void setNullMove(bool);

    void setMaxTimeMillsec(int);

    bool setParameter(String param, int value);

    int getMaxTimeMillsec();

    void startClock();

    int getRunning();

    _TpvLine &getPvLine() {
        return pvLine;
    }

    void setMainParam(const int depth);

    template<bool searchMoves>
    int search(const int depth, const int alpha, const int beta);

    void run();

    void endRun() { };
#ifndef JS_MODE
    void printDtmGtb();
    void printDtmSyzygy();
#endif

    void setMainPly(int);

    STATIC_CONST int NULL_DIVISOR = 6;
    STATIC_CONST int NULL_DEPTH = 3;
    STATIC_CONST int VAL_WINDOW = 50;

    void setRunningThread(bool t) {
        runningThread = t;
    }
    string probeRootTB();
    bool getRunningThread() const {
        return runningThread;
    }

//    void setValWindow(int valWin) {
//        Search::valWindow = valWin;
//    }

    int getValWindow() const {
        return valWindow;
    }

    void setChessboard(_Tchessboard &);

    _Tchessboard &getChessboard();

    u64 getZobristKey();

    int getMateIn();

#ifdef DEBUG_MODE

    unsigned totGen;

#endif

    void unsetSearchMoves();
    void setSearchMoves(vector<int> &v);
    void setHash(Hash *h) {
        hash = h;
    }

private:

    Hash *hash;

    vector<int> searchMovesVector;
    int valWindow = INT_MAX;
    static volatile bool runningThread;
    _TpvLine pvLine;

    bool ponder;

    template<bool searchMoves>
    void aspirationWindow(const int depth, const int valWindow);

    int checkTime();

    int maxTimeMillsec = 5000;
    bool nullSearch;
    static high_resolution_clock::time_point startTime;

    bool checkDraw(u64);

    template<int side, bool checkMoves>
    int search(int depth, int alpha, int beta, _TpvLine *pline, int N_PIECE, int *mateIn, int n_root_moves);
    int probeTB(const int side, const int N_PIECE, const int depth) const;

    template<bool checkMoves>
    bool checkSearchMoves(_Tmove *move);

    int probeGtb(const int side, const int N_PIECE, const int depth) const;

    bool checkInsufficientMaterial(int);

    void sortFromHash(const int listId, const Hash::_ThashData &phashe);

    template<int side>
    int quiescence(int alpha, int beta, const char promotionPiece, int, int depth);

    void updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move);

//    int getDtm1(const int side, _TpvLine *pline, const int depth, const int nPieces) const;

    int mainMateIn;
    int mainDepth;

};


