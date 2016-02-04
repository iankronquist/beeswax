package main

import (
	"github.com/iankronquist/senior-project-experiment/src/configurator"
	_ "github.com/iankronquist/senior-project-experiment/src/filter"
	_ "github.com/iankronquist/senior-project-experiment/src/monitor"
	_ "github.com/iankronquist/senior-project-experiment/src/reporter"
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
