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

