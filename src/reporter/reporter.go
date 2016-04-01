package reporter

import (
	"fmt"
	"github.com/fw42/go-hpfeeds"
	"sync"
	"time"
)

// Taken from http://blog.golang.org/pipelines
// Modified to support chan []byte instead of <-chan int
func merge(cs ...chan []byte) chan []byte {
	var wg sync.WaitGroup
	out := make(chan []byte)

	// Start an output goroutine for each input channel in cs.  output
	// copies values from c to out until c is closed, then calls wg.Done.
	output := func(c <-chan []byte) {
		for n := range c {
			out <- n
		}
		wg.Done()
	}
	wg.Add(len(cs))
	for _, c := range cs {
		go output(c)
	}

	// Start a goroutine to close out once all the output goroutines are
	// done.  This must start after the wg.Add call.
	go func() {
		wg.Wait()
		close(out)
	}()
	return out
}

func Start(host string, port int, ident string, auth string, outputs []chan []byte) {

	hp := hpfeeds.NewHpfeeds(host, port, ident, auth)
	hp.Log = true
	hp.Connect()

	// Publish something on "flotest" every second
	// Channel1 is where we put the filtered JSON data
	channel1 := merge(outputs...)
	hp.Publish("beeswax.events", channel1)
	go func() {
		for {
			channel1 <- []byte("Something")
			time.Sleep(time.Second)
		}
	}()

	// Subscribe to "flotest" and print everything coming in on it
	// prints something once every second - verify with others ::
	channel2 := make(chan hpfeeds.Message)
	hp.Subscribe("beeswax.events", channel2)
	go func() {
		for foo := range channel2 {
			fmt.Println(foo.Name, string(foo.Payload))
		}
	}()

	// Wait for disconnect
	<-hp.Disconnected
}
