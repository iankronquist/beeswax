package monitor

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"os"
	"os/exec"
)

/* The Monitor interface defines a series of methods which will be defined on
 * monitor structs. The Start method takes a channel to send messages over,
 * back to the configurator. The Stop method kills the process which is
 * performing the actual monitoring.
 */
type Monitor interface {
	Start(messages chan<- string)
	Stop()
}

/* Store the MonitorName, which is the name of the program to execute, the
 * DockerDir which is the directory to monitor which our Docker containers of
 * interest live in, and a pointer to the exec.Cmd struct which describes the
 * running command.
 */
type FSMonitor struct {
	MonitorName   string
	DockerDir     string
	fsWatcherProc *exec.Cmd
}

/* This function is going to need some comments describing why we chose this
 * approach, because it will be hairy.
 */
func (m FSMonitor) getDockerFSDirectory() string {
	// Sibi is writing this bit
	return "/"
}

/* Start the process running on the honeypot host to monitor the Docker
 * container. The Docker container's filesysem is mounted on the host. Find
 * the location of this filesysem with the getDockerFSDirectory function and
 * store it in the struct. Then create and start the process and forward
 * the output of the process on to the messages channel.
 */
func (m FSMonitor) Start(messages chan<- string) {
	fmt.Println(m.MonitorName)
	m.DockerDir = m.getDockerFSDirectory()
	m.fsWatcherProc = exec.Command(m.MonitorName, m.DockerDir)
	defer m.fsWatcherProc.Wait()

	outpipe, err := m.fsWatcherProc.StdoutPipe()
	if err != nil {
		log.Println("Could not open the ", m.MonitorName, "stdout pipe")
		panic(err)
		return
	}
	stderr, err := m.fsWatcherProc.StderrPipe()
	if err != nil {
		log.Println("Could not open the ", m.MonitorName, "stderr pipe")
		panic(err)
		return
	}
	go io.Copy(os.Stderr, stderr)

	m.fsWatcherProc.Start()
	fmt.Println(m.MonitorName)

	stdoutReader := bufio.NewReader(outpipe)
	for {
		line, _, err := stdoutReader.ReadLine()
		if err != nil {
			panic(err)
		}
		sb := string(line)
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
