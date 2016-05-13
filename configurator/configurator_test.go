package configurator

import (
	"fmt"
	"reflect"
	"testing"
)

func TestNonExistentConfigFile(t *testing.T) {
	_, err := ReadConfig("./testFixtures/non-existent.json")
	fmt.Println(err)
	if err == nil {
		t.Error("Expected error opening non-existent file, got nil")
	}
}

func TestMalformedConfigFile(t *testing.T) {
	_, err := ReadConfig("./testFixtures/honeypot_config_malformed.json")
	fmt.Println(err)
	if err == nil {
		t.Error("Expected error unmarshalling malformed json, got nil.")
	}
}

func TestCorrectConfigFile(t *testing.T) {
	expected := Config{
		DockerComposeName: "docker-compose",
		Containers:        []string{"wordpress", "mysql"},
	}
	config, err := ReadConfig("./testFixtures/honeypot_config.json")
	if err != nil {
		t.Error("Expected error to be nil, got", err)
	}
	if reflect.DeepEqual(expected, config) {
		t.Error("Expected config to be", expected, "instead got", config)
	}
}
