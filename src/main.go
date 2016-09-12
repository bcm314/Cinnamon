package main

import "fmt"

func main() {
	fmt.Printf("hello world");
	var a ChessBoard;
	a.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	a.display();
}
