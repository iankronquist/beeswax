CC=gcc

all: beeswax c_monitor_agents/znotify c_monitor_agents/proc

beeswax: deps main.go monitor/*.go filter/*.go reporter/*.go configurator/*.go
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
	go test ./configurator
	go test ./monitor
	go test ./filter
	go test ./reporter

clean:
	rm -f beeswax c_monitor_agents/znotify \
		c_monitor_agents/test_fs_monitor

.PHONY: deps clean all test
