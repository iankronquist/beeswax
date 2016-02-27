package monitor

import (
	"bufio"
	"fmt"
	"io"
	_ "io/ioutil"
	"log"
	"os"
	"os/exec"
	_ "regexp"
)

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

func getDockerContainerIds(dockerComposeName string) []string {
	if len(dockerContainerIds) != 0 {
		return dockerContainerIds
	}
	dockerComposeCommand := exec.Command(dockerComposeName, "ps", "-q")
	outpipe, err := dockerComposeCommand.StdoutPipe()
	if err != nil {
		panic(err)
	}

	dockerComposeCommand.Start()
	defer dockerComposeCommand.Wait()

	ids := []string{}
	stdoutReader := bufio.NewReader(outpipe)
	for {
		line, _, err := stdoutReader.ReadLine()
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		if string(line) != "" {
			ids = append(ids, string(line))
		}
	}
	dockerContainerIds = ids
	return ids
}

func (n NetMonitor) Start(messages chan<- []byte, dockerComposeName string) {
	ids := getDockerContainerIds(dockerComposeName)
	arguments := []string{"inspect", "-f", "'{{ .State.Pid }}'"}
	arguments = append(arguments, ids...)
	dockerComposeCommand := exec.Command("docker", arguments...)
	outpipe, err := dockerComposeCommand.StdoutPipe()
	if err != nil {
		panic(err)
	}

	dockerComposeCommand.Start()
	defer dockerComposeCommand.Wait()

	procIds := []string{}
	stdoutReader := bufio.NewReader(outpipe)
	for {
		line, _, err := stdoutReader.ReadLine()
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		if string(line) != "" {
			procIds = append(procIds, string(line))
			err = setSymlink(string(line), "/var/run/netns/")
			if err != nil {
				panic(err)
			}
			go startIPProcess(messages, string(line), "tcpdump", "")
		}
	}
}

func startIPProcess(messages chan<- []byte, procId string, watcherName string,
	watcherArgs ...string) {
	arguments := []string{"netns", "exec", procId, watcherName}
	arguments = append(arguments, watcherArgs...)
	netWatcherCommand := exec.Command("ip", arguments...)
	outpipe, err := netWatcherCommand.StdoutPipe()
	if err != nil {
		panic(err)
	}

	netWatcherCommand.Start()
	defer netWatcherCommand.Wait()

	stdoutReader := bufio.NewReader(outpipe)
	for {
		line, _, err := stdoutReader.ReadLine()
		if err == io.EOF {
			break
		} else if err != nil {
			panic(err)
		}
		messages <- line
	}
}

func setSymlink(procId string, destination string) error {
	err := os.Symlink("/proc/"+procId+"/ns/net", "/var/run/netns/"+procId)
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
 * container. The Docker container's filesysem is mounted on the host. Find
 * the location of this filesysem with the getDockerFSDirectory function and
 * store it in the struct. Then create and start the process and forward
 * the output of the process on to the messages channel.
 */
func (m FSMonitor) Start(messages chan<- []byte, dockerComposeName string) {
	m.DockerDirs = m.getDockerFSDirectory(dockerComposeName)
	m.fsWatcherProc = exec.Command(m.MonitorName, m.DockerDirs...)
	fmt.Println(m.MonitorName, m.DockerDirs)
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
		line, _, err := stdoutReader.ReadLine()
		if err != nil {
			panic(err)
		}
		sb := line
		messages <- sb

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
