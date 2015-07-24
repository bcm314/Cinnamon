/*
    Cinnamon is a UCI chess engine
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

#include "Uci.h"

Uci::Uci(Perft *perft1) {
    perft = perft1;
    perft->registerObservers([this]() {
        exit(0);//workaround
    });
    startListner();
}

void Uci::startListner() {
    iterativeDeeping = new IterativeDeeping();
    listner(iterativeDeeping);
}

Uci::Uci() {
    startListner();
}

Uci::~Uci() {
    delete iterativeDeeping;
}

void Uci::getToken(istringstream &uip, String &token) {
    token.clear();
    uip >> token;
    token.toLower();
}

void Uci::listner(IterativeDeeping *it) {
    string command;
    bool knowCommand;
    String token;
    bool stop = false;
    int lastTime = 0;
    uciMode = false;

    static const string BOOLEAN[] = {"false", "true"};
    while (!stop) {
        getline(cin, command);
        istringstream uip(command, ios::in);
        getToken(uip, token);
        knowCommand = false;
        if (token == "flush") {
            if (perft) {
                perft->dump();
            }
            knowCommand = true;
        } else if (token == "perft") {
            int perftDepth = -1;
            int nCpu = 1;
            string dumpFile;
            int PERFT_HASH_SIZE = 0;
            string fen;
            getToken(uip, token);
            while (!uip.eof()) {
                if (token == "depth") {
                    getToken(uip, token);
                    perftDepth = stoi(token);
                    if (perftDepth > MAX_PLY || perftDepth <= 0) {
                        perftDepth = 2;
                    }
                    getToken(uip, token);
                } else if (token == "ncpu") {
                    getToken(uip, token);
                    nCpu = stoi(token);
                    if (nCpu > 32 || nCpu <= 0) {
                        nCpu = 1;
                    }
                    getToken(uip, token);
                } else if (token == "dumpfile") {
                    getToken(uip, token);
                    dumpFile = token;
                    getToken(uip, token);
                } else if (token == "hash_size") {
                    getToken(uip, token);
                    PERFT_HASH_SIZE = stoi(token);
                    getToken(uip, token);
                } else if (token == "fen") {
                    uip >> token;
                    do {
                        fen += token;
                        fen += ' ';
                        uip >> token;
                    } while (token != "ncpu" && token != "hash_size" && token != "depth" && !uip.eof());
                } else {
                    break;
                }
            }
            if (perftDepth != -1) {
                int hashDepth = searchManager.getHashSize();
                searchManager.setHashSize(0);
                if (fen.empty()) {
                    fen = searchManager.getFen();
                }
                searchManager.setHashSize(hashDepth);
                //cout << "perft depth " << perftDepth << " nCpu " << nCpu << " hash_size " << PERFT_HASH_SIZE << " fen " << fen << " dumpFile '" << dumpFile << "'\n";
                perft = new Perft(fen, perftDepth, nCpu, PERFT_HASH_SIZE, dumpFile);
                perft->registerObservers([this]() {
                    delete perft;
                    perft = nullptr;
                });
                perft->start();
            } else {
                cout << "use: perft depth d [nCpu n] [hash_size mb] [fen fen_string] [dumpFile file_name]\n";
            }
            knowCommand = true;
        } else if (token == "quit") {
            knowCommand = true;
            searchManager.setRunning(false);
            it->join();
            stop = true;
        } else if (token == "ponderhit") {
            knowCommand = true;
            searchManager.startClock();
            searchManager.setMaxTimeMillsec(lastTime - lastTime / 3);
            searchManager.setPonder(false);
        } else if (token == "display") {
            knowCommand = true;
            searchManager.display();
        } else if (token == "isready") {
            knowCommand = true;
            searchManager.setRunning(0);
            cout << "readyok\n";
        } else if (token == "uci") {
            knowCommand = true;
            uciMode = true;
            cout << "id name " << NAME << "\n";
            cout << "id author Giuseppe Cannella\n";
            cout << "option name Hash type spin default 64 min 1 max 32768\n";
            cout << "option name Clear Hash type button\n";
            cout << "option name Nullmove type check default true\n";
            cout << "option name Book File type string default cinnamon.bin\n";
            cout << "option name OwnBook type check default " << BOOLEAN[it->getUseBook()] << "\n";
            cout << "option name Ponder type check default " << BOOLEAN[it->getPonderEnabled()] << "\n";
            cout << "option name Threads type spin default 1 min 1 max " << ThreadPool<void *>::MAX_THREAD << "\n";
            cout << "option name TB Endgame type combo default none var Gaviota var none\n";
            cout << "option name GaviotaTbPath type string default gtb/gtb4\n";
            cout << "option name GaviotaTbCache type spin default 32 min 1 max 1024\n";
            cout << "option name GaviotaTbScheme type combo default cp4 var none var cp1 var cp2 var cp3 var cp4\n";
            cout << "option name TB Pieces installed type combo default 3 var none var 3 var 4 var 5\n";
            cout << "option name TB probing depth type spin default 0 min 0 max 5\n";
            cout << "option name TB Restart type button\n";
            cout << "uciok\n";
        } else if (token == "score") {
            int side = searchManager.getSide();
            int t = searchManager.getScore(side);
            if (!searchManager.getSide()) {
                t = -t;
            }
            cout << "Score: " << t << "\n";
            knowCommand = true;
        } else if (token == "stop") {
            knowCommand = true;
            searchManager.setPonder(false);
            searchManager.setRunning(0);
        } else if (token == "ucinewgame") {
            searchManager.loadFen();
            searchManager.clearHash();
            knowCommand = true;
        } else if (token == "setvalue") {
            getToken(uip, token);
            String value;
            getToken(uip, value);
            knowCommand = it->setParameter(token, stoi(value));
        } else if (token == "setoption") {
            getToken(uip, token);
            if (token == "name") {
                getToken(uip, token);
                if (token == "gaviotatbpath") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        it->createGtb();
                        tablebase->setPath(token);
                    }
                } else if (token == "gaviotatbcache") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        if (tablebase->setCacheSize(stoi(token))) {
                            knowCommand = true;
                        };
                    }
                } else if (token == "threads") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        if (searchManager.setThread(stoi(token))) {
                            knowCommand = true;
                        };
                    }
                } else if (token == "gaviotatbscheme") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        if (tablebase->setScheme(token)) {
                            knowCommand = true;
                        };
                    }
                } else if (token == "tb") {
                    getToken(uip, token);
                    if (token == "pieces") {
                        getToken(uip, token);
                        if (token == "installed") {
                            getToken(uip, token);
                            if (token == "value") {
                                getToken(uip, token);
                                if (tablebase->setInstalledPieces(stoi(token))) {
                                    knowCommand = true;
                                };
                            }
                        }
                    } else if (token == "endgame") {
                        getToken(uip, token);
                        if (token == "value") {
                            getToken(uip, token);
                            knowCommand = true;
                            if (token == "none") {
                                searchManager.deleteGtb();
                                knowCommand = true;
                            } else if (token == "gaviota") {
                                //it->getGtb();
                                knowCommand = true;
                            }
//                            else {
//                                knowCommand = false;
//                            }
                        }
                    } else if (token == "restart") {
                        knowCommand = true;
                        tablebase->restart();
                    } else if (token == "probing") {
                        getToken(uip, token);
                        if (token == "depth") {
                            getToken(uip, token);
                            if (token == "value") {
                                getToken(uip, token);
                                if (tablebase->setProbeDepth(stoi(token))) {
                                    knowCommand = true;
                                };
                            }
                        }
                    }
                } else if (token == "hash") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        if (searchManager.setHashSize(stoi(token))) {
                            knowCommand = true;
                        };
                    }
                } else if (token == "nullmove") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        searchManager.setNullMove(token == "true");
                    }
                } else if (token == "ownbook") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        it->setUseBook(token == "true");
                        knowCommand = true;
                    }
                } else if (token == "book") {
                    getToken(uip, token);
                    if (token == "file") {
                        getToken(uip, token);
                        if (token == "value") {
                            getToken(uip, token);
                            it->loadBook(token);
                            knowCommand = true;
                        }
                    }
                } else if (token == "ponder") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        it->enablePonder(token == "true");
                        knowCommand = true;
                    }
                } else if (token == "clear") {
                    getToken(uip, token);
                    if (token == "hash") {
                        knowCommand = true;
                        searchManager.clearHash();
                    }
                }
            }
        } else if (token == "position") {
            lock_guard<mutex> lock(it->mutexIT);
            knowCommand = true;
            searchManager.setRepetitionMapCount(0);
            getToken(uip, token);
            _Tmove move;
            if (token == "startpos") {
                it->setUseBook(it->getUseBook());
                searchManager.loadFen();
                getToken(uip, token);
            }
            if (token == "fen") {
                string fen;
                while (token != "moves" && !uip.eof()) {
                    uip >> token;
                    fen += token;
                    fen += ' ';
                }
                searchManager.init();
                int x = searchManager.loadFen(fen);
                searchManager.setSide(x);
                searchManager.pushStackMove();
            }
            if (token == "moves") {
                while (!uip.eof()) {
                    uip >> token;
                    int x = !searchManager.getMoveFromSan(token, &move);
                    searchManager.setSide(x);
                    searchManager.makemove(&move);
                }
            }
        } else if (token == "go") {
            it->setMaxDepth(MAX_PLY);
            int wtime = 200000; //5 min
            int btime = 200000;
            int winc = 0;
            int binc = 0;
            bool forceTime = false;
            bool setMovetime = false;
            while (!uip.eof()) {
                getToken(uip, token);
                if (token == "wtime") {
                    uip >> wtime;
                } else if (token == "btime") {
                    uip >> btime;
                } else if (token == "winc") {
                    uip >> winc;
                } else if (token == "binc") {
                    uip >> binc;
                } else if (token == "depth") {
                    int depth;
                    uip >> depth;
                    if (depth > MAX_PLY) {
                        depth = MAX_PLY;
                    }
                    if (!setMovetime) {
                        searchManager.setMaxTimeMillsec(0x7FFFFFFF);
                    }
                    it->setMaxDepth(depth);
                    forceTime = true;
                } else if (token == "movetime") {
                    int tim;
                    uip >> tim;
                    setMovetime = true;
                    searchManager.setMaxTimeMillsec(tim);
                    forceTime = true;
                } else if (token == "infinite") {
                    searchManager.setMaxTimeMillsec(0x7FFFFFFF);
                    forceTime = true;
                } else if (token == "ponder") {
                    searchManager.setPonder(true);
                }
            }
            if (!forceTime) {
                if (searchManager.getSide() == WHITE) {
                    winc -= (int) (winc * 0.1);
                    searchManager.setMaxTimeMillsec(winc + wtime / 40);
                    if (btime > wtime) {
                        searchManager.setMaxTimeMillsec(searchManager.getMaxTimeMillsec() - (int) (searchManager.getMaxTimeMillsec() * ((135.0 - wtime * 100.0 / btime) / 100.0)));
                    }
                } else {
                    binc -= (int) (binc * 0.1);
                    searchManager.setMaxTimeMillsec(binc + btime / 40);
                    if (wtime > btime) {
                        searchManager.setMaxTimeMillsec(searchManager.getMaxTimeMillsec() - (int) (searchManager.getMaxTimeMillsec() * ((135.0 - btime * 100.0 / wtime) / 100.0)));
                    }
                }
                lastTime = searchManager.getMaxTimeMillsec();
            }
            if (!uciMode) {
                searchManager.display();
            }
            it->start();
            it->detach();
            knowCommand = true;
        }
        if (!knowCommand) {
            cout << "Unknown command: " << command << "\n";
        };
        cout << flush;
    }
}
