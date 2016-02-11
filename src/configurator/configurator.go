package configurator

import (
	"encoding/json"
	"fmt"
	"github.com/iankronquist/senior-project-experiment/src/monitor"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
)

type Config struct {
	DockerComposeName string   `json:"docker compose name"`
	MonitorName       string   `json:"monitor process name"`
	Containers        []string `json:"container names"`
}

func StartContainers(c Config) error {

	for _, containerName := range c.Containers {
		containerProc := exec.Command(c.DockerComposeName, "up", "-d", containerName)

		fmt.Println(c.DockerComposeName, "up", "-d", containerName)

		// Copy docker-compose stdout/stderr to configurator's
		compose_stderr, err := containerProc.StderrPipe()
		if err != nil {
			panic(err)
		}
		compose_stdout, err := containerProc.StdoutPipe()
		if err != nil {
			panic(err)
		}
		go io.Copy(os.Stdout, compose_stdout)
		go io.Copy(os.Stderr, compose_stderr)

		containerProc.Start()

		err = containerProc.Wait()
		if err != nil {
			fmt.Println(err)
			return err
		}
	}
	fmt.Println("Containers started!")
	return nil
}

func ReadConfig(fileName string) (Config, error) {
	config := Config{}
	data, err := ioutil.ReadFile(fileName)
	if err != nil {
		return config, err
	}
	err = json.Unmarshal(data, &config)
	if err != nil {
		return config, err
	}
	return config, nil
}

func StartMonitor(c Config) {
	FSMessages := make(chan string)
	//networkMessages := make(chan string)
	//execMessages := make(chan string)
	fsMonitor := monitor.FSMonitor{MonitorName: c.MonitorName}

	go fsMonitor.Start(FSMessages, c.DockerComposeName)
	for {
		select {
			case message := <-FSMessages:
				fmt.Println("Message received: ", message)
		}
	}
	/*
		go monitor.StartNetWorkMonitor(networkMessages)
		go monitor.StartExecMonitor(execMessages)
	*/
}
