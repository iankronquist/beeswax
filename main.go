package main

import (
	"github.com/iankronquist/beeswax/configurator"
)

func main() {
	config, err := configurator.ReadConfig("./honeypot_config.json")
	if err != nil {
		panic(err)
	}
	err = configurator.StartContainers(config)
	if err != nil {
		panic(err)
	}
	configurator.StartMonitor(config)
}
