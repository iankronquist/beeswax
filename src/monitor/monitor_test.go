package monitor

import "testing"
import "reflect"

/*
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
*/

func TestRunCommandAndSlurpOutput( t *testing.T) {
	t.Fail()
	t.Error("BOOGERS")
	command := "echo"
	arguments := []string{}
	expected := []string{}
	actual,err := runCommandAndSlurpOutput(command,arguments)
	if err != nil {
		t.Error("Echo Errored")
	}
	if !reflect.DeepEqual(actual, expected) {
		t.Error("Expected Nothing but Got:",actual)	
	}
}
