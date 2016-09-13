package main

import "fmt"
import "time"
func main() {
	fmt.Printf("\n\nhello world");
	var a ChessBoard;
	a.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	//time.Sleep(time.Second * 2)
	a.display();
	time.Sleep(time.Second * 2)
}
