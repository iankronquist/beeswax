package main

import (
	"github.com/iankronquist/beeswax/src/configurator"
	_ "github.com/iankronquist/beeswax/src/filter"
	_ "github.com/iankronquist/beeswax/src/monitor"
	_ "github.com/iankronquist/beeswax/src/reporter"
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
