package configurator

import (
	"encoding/json"
	"fmt"
	"github.com/iankronquist/beeswax/filter"
	"github.com/iankronquist/beeswax/monitor"
	"github.com/iankronquist/beeswax/reporter"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
)

type Config struct {
	DockerComposeName string   `json:"docker compose name"`
	MonitorName       string   `json:"monitor process name"`
	Containers        []string `json:"container names"`
	MHNHost           string   `json:"mhn host"`
	MHNPort           int      `json:"mhn port"`
	MHNIdent          string   `json:"mhn identifier"`
	MHNAuth           string   `json:"mhn authorization"`
}

func StartContainers(c Config) error {

	for _, containerName := range c.Containers {
		containerProc := exec.Command(c.DockerComposeName, "-f", "containers/docker-compose.yml", "up", "-d", containerName)

		fmt.Println(c.DockerComposeName, "-f", "containers/docker-compose.yml", "up", "-d", containerName)

		// Copy docker-compose stdout/stderr to configurator's
		composeStderr, err := containerProc.StderrPipe()
		if err != nil {
			panic(err)
		}
		composeStdout, err := containerProc.StdoutPipe()
		if err != nil {
			panic(err)
		}
		go io.Copy(os.Stdout, composeStdout)
		go io.Copy(os.Stderr, composeStderr)

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
	FSMessages := make(chan []byte)
	FSMessagesOut := make(chan []byte)
	networkMessages := make(chan []byte)
	networkMessagesOut := make(chan []byte)
	execMessages := make(chan []byte)
	reporterMessages := make(chan []byte)
	netMonitor := monitor.NetMonitor{}
	fsMonitor := monitor.FSMonitor{MonitorName: c.MonitorName}
	execMonitor := monitor.ExecMonitor{}
	go netMonitor.Start(networkMessages, c.DockerComposeName)
	go fsMonitor.Start(FSMessages, c.DockerComposeName)
	go execMonitor.Start(execMessages, c.DockerComposeName)
	go filter.StartFilterStream(FSMessagesOut, FSMessages)
	go filter.StartNetFilterStream(networkMessagesOut, networkMessages)
	go reporter.Start(c.MHNHost, c.MHNPort, c.MHNIdent, c.MHNAuth, reporterMessages)
	for {
		select {
		case message, err := <-execMessages:
			if err {
				fmt.Println("exec: ", string(message))
				go func() { reporterMessages <- message }()
			}
		case message, err := <-networkMessagesOut:
			if err {
				fmt.Println("net: ", string(message))
				go func() { reporterMessages <- message }()
			}
		case message, err := <-FSMessagesOut:
			if err {
				fmt.Println("filtered fs: ", string(message))
				go func() { reporterMessages <- message }()
			}
		}
	}

}
