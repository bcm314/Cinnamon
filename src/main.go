package main

import "fmt"

func main() {
	fmt.Printf("\n\nhello world");
	a:= NewGenMoves();
	a.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	a.display();

}
