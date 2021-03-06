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

#include <mutex>
#include "Hash.h"

Hash::Hash() {
    HASH_SIZE = 0;
    hashArray[HASH_ALWAYS] = hashArray[HASH_GREATER] = nullptr;
#ifdef DEBUG_MODE
    n_cut_hashA = n_cut_hashB = cutFailed = probeHash = readCollisions = 0;
    nRecordHashA = nRecordHashB = nRecordHashE = collisions = 0;
#endif
    setHashSize(HASH_SIZE_DEFAULT);

}

void Hash::clearAge() {
    for (int i = 0; i < HASH_SIZE; i++) {
        hashArray[HASH_ALWAYS][i].u.dataS.entryAge = 0;
    }
}

void Hash::clearHash() {
    if (!HASH_SIZE) {
        return;
    }
    memset(static_cast<void*>(hashArray[HASH_GREATER]), 0, sizeof(_Thash) * HASH_SIZE);
    memset(static_cast<void*>(hashArray[HASH_ALWAYS]), 0, sizeof(_Thash) * HASH_SIZE);
}

int Hash::getHashSize() const {
    return HASH_SIZE / (1024 * 1000 / (sizeof(_Thash) * 2));
}

void Hash::setHashSize(int mb) {
    dispose();
    if (mb > 0) {
        int tmp = mb * 1024 * 1000 / (sizeof(_Thash) * 2);
        hashArray[HASH_ALWAYS] = (_Thash *) calloc(tmp, sizeof(_Thash));
        if (!hashArray[HASH_ALWAYS]) {
            fatal("info string error - no memory");
            exit(1);
        }
        hashArray[HASH_GREATER] = (_Thash *) calloc(tmp, sizeof(_Thash));
        if (!hashArray[HASH_GREATER]) {
            fatal("info string error - no memory");
            exit(1);
        }
        HASH_SIZE = tmp;
    }
}

void Hash::dispose() {
    if (hashArray[HASH_ALWAYS] != nullptr) {
        free(hashArray[HASH_ALWAYS]);
    }
    if (hashArray[HASH_GREATER] != nullptr) {
        free(hashArray[HASH_GREATER]);
    }
    hashArray[HASH_ALWAYS] = hashArray[HASH_GREATER] = nullptr;
    HASH_SIZE = 0;
}

Hash::~Hash() {
    dispose();
}

