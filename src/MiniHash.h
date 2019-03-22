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

#include <stdlib.h>

class MiniHash {

private:
    u64 *hash;
    unsigned hashSize;
    static constexpr u64 keyMask = 0xffffffffffff0000ULL;
    static constexpr u64 valueMask = 0xffffULL;

public:
    static constexpr short noHashValue = 0x7fff;

    MiniHash(const unsigned size) {
        hashSize = size;
        hash = (u64 *) calloc(size, sizeof(u64));
    }

    ~MiniHash() {
        free(hash);
        hash = nullptr;
    }

    inline void recordHash(const u64 key, const short value) {
        hash[key % hashSize] = (key & keyMask) | (value & valueMask);
        ASSERT(value == readHash(key));
    }

    inline short readHash(const u64 key) const {
        const u64 kv = hash[key % hashSize];
        if ((kv & keyMask) == (key & keyMask)) return kv;
        return noHashValue;
    }

};
