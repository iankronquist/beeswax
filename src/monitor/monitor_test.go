package monitor

import "testing"
import "reflect"
//import "strings"

type testCommandData struct{
	command	  string
	arguments []string
	expected  []string
}

func TestRunCommandAndSlurpOutput( t *testing.T) {
		
	testArray := []testCommandData{{"echo",[]string{},[]string{}},
				{"echo",[]string{"hello"},[]string{"hello"}},
				{"echo",[]string{"-e","Hello\nWorld"},[]string{"Hello","World"}}}
	for _, item := range(testArray) {
		actual,err := runCommandAndSlurpOutput(item.command,item.arguments)
		if err != nil {
			t.Error("Echo Errored")
		}
		if !reflect.DeepEqual(actual, item.expected) {
			t.Error("Expected Nothing but Got:",actual)	
		}
	}
}

func TestRunCommandAndChannelOutput( t *testing.T) {
		
	testArray := []testCommandData{{"echo",[]string{},[]string{}},
				{"echo",[]string{"hello"},[]string{"hello"}},
				{"echo",[]string{"-e","Hello\nWorld"},[]string{"Hello","World"}}}
	for _, item := range(testArray) {
		c := make( chan []byte)
		go runCommandAndChannelOutput(item.command,item.arguments,c)
		i := 0
		for instance := range c {
			if item.expected[i] != string(instance) {
				t.Errorf("Expected \"%s\" but got \"%s\"",item.expected[i],string(instance))
			}
			i++
		}
	}
}

