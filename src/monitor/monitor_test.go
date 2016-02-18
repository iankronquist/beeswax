package monitor

import "testing"
import "fmt"

func TestFSMonitorStart(t *testing.T) {
	fsMonitor := FSMonitor{MonitorName: "cstuff/test_fs_monitor"}
	channel := make(chan string)
	err := fsMonitor.Start(channel)
	for {
		b := <-channel
		fmt.Println(b)
		t.Fail()
	}
}

/*
func (m FSMonitor) Start(messages chan<-string) {
	m.DockerDir = m.getDockerFSDirectory()
	m.fsWatcherProc = exec.Command(m.MonitorName, m.DockerDir)
	m.fsWatcherProc.Start()
	outpipe, err := m.fsWatcherProc.StdoutPipe()
	if err != nil {
		log.Println("Could not open the ", m.MonitorName, "stdout pipe")
		return
	}
	scanner := bufio.NewScanner(outpipe)
	for scanner.Scan() {
		messages <- scanner.Text()
	}
	if err := scanner.Err(); err != nil {
		log.Println("FS Monitor ", m.MonitorName, "exited with an error", err)
	}
}
*/

/*
func (m FSMonitor) Stop() {
	err := m.fsWatcherProc.Process.Kill()
	if err != nil {
		log.Fatal("Could not kill ", m.MonitorName, err)
	}

}
*/
