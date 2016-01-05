package main

import (
	"configurator"
	_ "filter"
	_ "monitor"
	_ "reporter"
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
}
