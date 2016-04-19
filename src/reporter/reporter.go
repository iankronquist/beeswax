package reporter

import (
	"fmt"
	"github.com/fw42/go-hpfeeds"
)

func Start(host string, port int, ident string, auth string, outputs chan []byte) {
	fmt.Printf("'%v', '%v', '%v', '%v'\n", host, port, ident, auth)
	hp := hpfeeds.NewHpfeeds(host, port, ident, auth)
	hp.Log = true

	fmt.Println("Waiting to connect to MHN server...")
	err := hp.Connect()
	if err != nil {
		fmt.Println("Failed to connect to MHN server!")
		panic(err)
	}
	fmt.Println("Successfully connected to MHN server!")

	// Publish something on "beeswax.events" every second
	hp.Publish("beeswax.events", outputs)

	// Wait for disconnect
	<-hp.Disconnected
}
