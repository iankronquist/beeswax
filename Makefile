all: senior-project-experiment

senior-project-experiment: main.go src/
	go build

test:
	go test ./src/configurator
	go test ./src/monitor
	go test ./src/reporter
	go test ./src/filter
