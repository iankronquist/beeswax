package reporter

import (
	"github.com/fw42/go-hpfeeds"
)

func StartReporter(host string, port int, ident, auth string, channels []chan []byte) {
	hp := hpfeeds.NewHpfeeds(host, port, ident, auth)
	hp.Log = true
	hp.Connect()
	for _, channel := range channels {
		hp.Publish("", channel)
	}

}
