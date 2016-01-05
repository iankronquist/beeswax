package main

import (
	"configurator"
	_ "filter"
	_ "monitor"
	_ "reporter"
)

func main() {
	config, err := configurator.ReadConfig()
	if err != nil {
		panic(err)
	}
	err = configurator.StartContainers(config)
	if err != nil {
		panic(err)
	}
}
