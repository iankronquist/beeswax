CC=gcc

all: beeswax c_monitor_agents/znotify c_monitor_agents/proc

beeswax: deps main.go src/monitor/*.go src/filter/*.go src/reporter/*.go src/configurator/*.go
	go build

c_monitor_agents/test_fs_monitor: c_monitor_agents/test_fs_monitor.c
	${CC} -o c_monitor_agents/test_fs_monitor c_monitor_agents/test_fs_monitor.c

c_monitor_agents/znotify: c_monitor_agents/znotify.c
	${CC} -o c_monitor_agents/znotify c_monitor_agents/znotify.c

c_monitor_agents/proc: c_monitor_agents/proc.c
	${CC} -o c_monitor_agents/proc c_monitor_agents/proc.c -lm

deps:
	go get

test: deps c_monitor_agents/test_fs_monitor
	go test ./src/configurator
	go test ./src/monitor
	go test ./src/filter
	go test ./src/reporter

clean:
	rm -f beeswax c_monitor_agents/znotify \
		c_monitor_agents/test_fs_monitor

.PHONY: deps clean all test
