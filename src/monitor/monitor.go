package monitor

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	_ "io/ioutil"
	"log"
	"os"
	"os/exec"
	"regexp"
	"strings"
)

type ipdata struct {
	Epoch       string `json:"Epoch"`
	SourceIP    string `json:"SourceIP"`
	SourcePort  string `json:"SourcePort"`
	ReceiveIP   string `json:"ReceiveIP"`
	ReceivePort string `json:"ReceivePort"`
}

/* The Monitor interface defines a series of methods which will be defined on
 * monitor structs. The Start method takes a channel to send messages over,
 * back to the configurator. The Stop method kills the process which is
 * performing the actual monitoring.
 */
type Monitor interface {
	Start(messages chan<- []byte, dockerComposeName string)
	Stop()
}

/* Store the MonitorName, which is the name of the program to execute, the
 * DockerDirs which are the directories to monitor which our Docker containers
 * of interest live in, and a pointer to the exec.Cmd struct which describes
 * the running command.
 */
type FSMonitor struct {
	MonitorName   string
	DockerDirs    []string
	fsWatcherProc *exec.Cmd
}

type NetMonitor struct {
	MonitorName  string
	ContainerIds []string
}

// Memoize this. It's kind of expensive to get.
var dockerContainerIds = []string{}

func runCommandAndSlurpOutput(commandname string, args []string) ([]string, error) {
	command := exec.Command(commandname, args...)
	fmt.Print("running the command: ")
	fmt.Println(commandname, args)
	stdout, err := command.StdoutPipe()
	if err != nil {
		return nil, err
	}
	stderr, err := command.StderrPipe()
	if err != nil {
		return nil, err
	}

	go io.Copy(os.Stderr, stderr)
	command.Start()
	defer command.Wait()

	output := []string{}
	stdoutreader := bufio.NewReader(stdout)
	slurp := true
	for slurp {
		fetch := true
		line := []byte{}
		for fetch {
			partial_line, f, err := stdoutreader.ReadLine()
			fetch = f
			line = append(line, partial_line...)
			if err == io.EOF {
				slurp = false
				break
			} else if err != nil {
				return nil, err
			}
		}
		if len(line) > 0 {
			output = append(output, string(line))
		}
	}
	return output, nil
}

func runCommandAndChannelOutput(commandname string, args []string, output chan<- []byte) error {
	command := exec.Command(commandname, args...)
	fmt.Print("running the command: ")
	fmt.Println(commandname, args)
	stdout, err := command.StdoutPipe()
	if err != nil {
		return err
	}
	stderr, err := command.StderrPipe()
	if err != nil {
		return err
	}

	go io.Copy(os.Stderr, stderr)
	command.Start()
	defer command.Wait()

	stdoutreader := bufio.NewReader(stdout)
	slurp := true
	for slurp {
		fetch := true
		line := []byte{}
		for fetch {
			partial_line, f, err := stdoutreader.ReadLine()
			fetch = f
			line = append(line, partial_line...)
			if err == io.EOF {
				slurp = false
				break
			} else if err != nil {
				return err
			}
		}
		if len(line) > 0 {
			output <- line
		}
	}
	return nil
}

func getDockerContainerIds(dockerComposeName string) []string {
	if len(dockerContainerIds) != 0 {
		return dockerContainerIds
	}
	ids, err := runCommandAndSlurpOutput("docker-compose", []string{"ps", "-q"})
	if err != nil {
		panic(err)
	}
	dockerContainerIds = ids
	return ids
}

func (n NetMonitor) Start(messages chan<- []byte, dockerComposeName string) {

	ids := getDockerContainerIds(dockerComposeName)
	arguments := []string{"inspect", "-f", "'{{ .State.Pid }}'"}
	arguments = append(arguments, ids...)
	output, err := runCommandAndSlurpOutput("docker", arguments)
	if err != nil {
		panic(err)
	}
	nmProcessorChan := make(chan []byte)
	go networkMonitorProcessor(messages, nmProcessorChan)
	for _, procId := range output {
		// FIXME: do we want to throw an error instead?
		if procId == "'0'" {
			fmt.Println("Container isn't running")
			continue
		}
		// Replace all quotes
		scrubbedProcId := strings.Replace(procId, "'", "", -1)
		// TODO: Check for ids named 0, which means no proc id
		err := setSymlink(scrubbedProcId, scrubbedProcId)
		if err != nil {
			panic(err)
		}
		go startIPProcess(nmProcessorChan, scrubbedProcId, "tcpdump", "-tt", "-n", "-i", "any", "-l")
	}
}
func networkMonitorProcessor(sending chan<- []byte, receiving <-chan []byte) error {
	platform := []byte{}
	carlson := []byte{}
	pattern := "(\\d+\\.\\d+) IP (\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\.(\\d+) > (\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\.(\\d+)"
	compiled_pattern, err := regexp.Compile(pattern)
	if err != nil {
		return err
	}
	for {
		platform = <-receiving
		matches := compiled_pattern.FindSubmatch(platform)
		fmt.Println(string(matches[2]), string(matches[1]))
		carl := ipdata{
			Epoch:       string(matches[1]),
			SourceIP:    string(matches[2]),
			SourcePort:  string(matches[3]),
			ReceiveIP:   string(matches[4]),
			ReceivePort: string(matches[5]),
		}
		carlson, err = json.Marshal(carl)
		if err != nil {
			return err
		}
		fmt.Println("Sending json: ", string(carlson))
		sending <- carlson
	}

	return nil
}
func startIPProcess(messages chan<- []byte, procId string, watcherName string,
	watcherArgs ...string) {
	arguments := []string{"netns", "exec", procId, watcherName}
	arguments = append(arguments, watcherArgs...)
	err := runCommandAndChannelOutput("ip", arguments, messages)
	if err != nil {
		panic(err)
	}
}

func setSymlink(procId string, destination string) error {
	nameSpaceDir := "/var/run/netns/"
	target := nameSpaceDir + destination
	source := "/proc/" + procId + "/ns/net"

	err := os.MkdirAll(nameSpaceDir, 0777)
	if err != nil {
		return err
	}
	_, err = os.Stat(target)
	if err == nil {
		err = os.Remove(target)
		if err != nil {
			return err
		}
	}

	_, err = os.Stat(source)
	if os.IsNotExist(err) {
		return errors.New("Could not set symlink: Target directory " +
			target + " does not exist")
	}

	err = os.Symlink(source, target)
	return err
}

/* This function is going to need some comments describing why we chose this
 * approach, because it will be hairy.
 * FIXME: Break out functionality into multiple functions so we can test this
 * easily!
 */
func (m FSMonitor) getDockerFSDirectory(dockerComposeName string) []string {
	ids := getDockerContainerIds(dockerComposeName)
	/*
		dockerInfoCommand := exec.Command("docker", "info")
		infoOutPipe, err := dockerInfoCommand.StdoutPipe()

		if err != nil {
			panic(err)
		}

		dockerInfoCommand.Start()
		defer dockerInfoCommand.Wait()

		infoBuf, err := ioutil.ReadAll(infoOutPipe)

		if err != nil {
			panic(err)
		}

		info := string(infoBuf)

		re := regexp.MustCompile("Docker Root Dir: (.*)")

		submatch := re.FindStringSubmatch(info)

		if len(submatch) < 2 {
			fmt.Println(submatch, info)
			panic("Couldn't find the docker root directory")
		}
		dockerRootPath := submatch[1]
	*/
	dockerRootPath := "/var/lib/docker/aufs"

	for i := 0; i < len(ids); i++ {
		ids[i] = dockerRootPath + "/mnt/" + ids[i]
	}
	return ids
}

/* Start the process running on the honeypot host to monitor the Docker
 * container. The Docker container's filesystem is mounted on the host. Find
 * the location of this filesystem with the getDockerFSDirectory function and
 * store it in the struct. Then create and start the process and forward
 * the output of the process on to the messages channel.
 */
func (m FSMonitor) Start(messages chan<- []byte, dockerComposeName string) {
	m.DockerDirs = m.getDockerFSDirectory(dockerComposeName)
	// FIXME Make arguments configurable
	//arguments := append([]string{"-wte"}, m.DockerDirs...)
	arguments := m.DockerDirs
	fmt.Println(m.MonitorName, arguments)
	m.fsWatcherProc = exec.Command(m.MonitorName, arguments...)
	defer m.fsWatcherProc.Wait()

	outpipe, err := m.fsWatcherProc.StdoutPipe()
	if err != nil {
		log.Println("Could not open the ", m.MonitorName, "stdout pipe")
		panic(err)
	}
	stderr, err := m.fsWatcherProc.StderrPipe()
	if err != nil {
		log.Println("Could not open the ", m.MonitorName, "stderr pipe")
		panic(err)
	}
	go io.Copy(os.Stderr, stderr)

	m.fsWatcherProc.Start()

	stdoutReader := bufio.NewReader(outpipe)

	for {
		fetch := true
		line := []byte{}
		for fetch {
			partial_line, f, err := stdoutReader.ReadLine()
			if err != nil {
				fmt.Println("File monitor stopped")
				panic(err)
			}
			fetch = f
			line = append(line, partial_line...)
		}
		messages <- line

	}
}

/* Stop the filesystem monitor. Kill the process monitoring the Docker
 * container's filesysem.
 */
func (m FSMonitor) Stop() {
	err := m.fsWatcherProc.Process.Kill()
	if err != nil {
		log.Fatal("Could not kill ", m.MonitorName, err)
	}
}
