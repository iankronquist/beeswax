package main

import (
	"../"
	"fmt"
	"os"
	"time"
)

func main() {
	if len(os.Args) != 3 {
		fmt.Printf("Usage: %s <ident> <authkey>\n", os.Args[0])
		os.Exit(0)
	}

	host := "hpfriends.honeycloud.net"
	port := 20000
	ident := os.Args[1]
	auth := os.Args[2]

	hp := hpfeeds.NewHpfeeds(host, port, ident, auth)
	hp.Log = true
	hp.Connect()

	// Publish something on "flotest" every second
	channel1 := make(chan []byte)
	hp.Publish("flotest", channel1)
	go func() {
		for {
			channel1 <- []byte("Something")
			time.Sleep(time.Second)
		}
	}()

	// Subscribe to "flotest" and print everything coming in on it
	channel2 := make(chan hpfeeds.Message)
	hp.Subscribe("flotest", channel2)
	go func() {
		for foo := range channel2 {
			fmt.Println(foo.Name, string(foo.Payload))
		}
	}()

	// Wait for disconnect
	<-hp.Disconnected
}
