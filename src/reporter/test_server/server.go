package main

import (
	"fmt"
	"github.com/fw42/go-hpfeeds"
)

func main() {
	host := "hpfriends.honeycloud.net"
	port := 20000
	ident := "testing"
	auth := "t0ps3cr3t"

	hp := hpfeeds.NewHpfeeds(host, port, ident, auth)
	hp.Log = true

	recv := make(chan hpfeeds.Message)
	hp.Subscribe("testing", recv)
	go func() {
		for foo := range recv {
			fmt.Println(foo.Name, string(foo.Payload))
		}
	}()

	// Wait for disconnect
	<-hp.Disconnected
}
