package main

import "fmt"

func main() {

	a:= NewGenMoves();
	a.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	a.display();
	fmt.Print("end\n")
}
