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
	BLACK int = 0;
	WHITE int = 1;
)

type  _Tchessboard[16]uint64;

func RESET_LSB(bits uint64) uint64 {
	bits &= bits - 1; return bits;
}

func bitCount(bits uint64) int {
	count := 0;
	for ok := true; ok; ok = (bits != 0) {
		//while (bits) {
		count++;
		bits &= bits - 1;
	}
	return count;
}
func assert(a bool, b string) {
	if a == true {
		return
	}
	for {
		fmt.Printf("error %v\n",b)
	}
}
func BITScanForward(bb uint64) int {
	//  @author Matt Taylor (2003)
	lsb_64_table := []int{63, 30, 3, 32, 59, 14, 11, 33, 60, 24, 50, 9, 55, 19, 21, 34, 61, 29, 2, 53, 51, 23, 41, 18, 56, 28, 1, 43, 46, 27, 0, 35, 62, 31, 58, 4, 5, 49, 54, 6, 15, 52, 12, 40, 7, 42, 45, 16, 25, 57, 48, 13, 10, 39, 8, 44, 20, 47, 38, 22, 17, 37, 36, 26 };
	bb ^= bb - 1;
	var folded int = int(bb ^ (bb >> 32));
	return lsb_64_table[folded * 0x78291ACF >> 26];
}

func BITScanReverse(bb uint64) int {
	// authors Kim Walisch, Mark Dickinson
	index64 := []int{0, 47, 1, 56, 48, 27, 2, 60, 57, 49, 41, 37, 28, 16, 3, 61, 54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11, 4, 62, 46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10, 45, 25, 39, 14, 33, 19, 30, 9, 24, 13, 18, 8, 12, 7, 6, 5, 63, };
	const debruijn64 uint64 = 0x03f79d71b4cb0a89;
	bb |= bb >> 1;
	bb |= bb >> 2;
	bb |= bb >> 4;
	bb |= bb >> 8;
	bb |= bb >> 16;
	bb |= bb >> 32;
	return index64[(bb * debruijn64) >> 58];
}



