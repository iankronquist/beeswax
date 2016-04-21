CC=gcc

all: senior-project-experiment c_fs_monitor/znotify cproc_monitor/proc

senior-project-experiment: deps main.go src/monitor/*.go src/filter/*.go src/reporter/*.go src/configurator/*.go
	go build

c_fs_monitor/test_fs_monitor: c_fs_monitor/test_fs_monitor.c
	${CC} -o c_fs_monitor/test_fs_monitor c_fs_monitor/test_fs_monitor.c

c_fs_monitor/znotify: c_fs_monitor/znotify.c
	${CC} -o c_fs_monitor/znotify c_fs_monitor/znotify.c

cproc_monitor/proc: cproc_monitor/proc.c
	${CC} -o cproc_monitor/proc cproc_monitor/proc.c -lm

deps:
	go get

test: deps c_fs_monitor/test_fs_monitor
	go test ./src/configurator
	go test ./src/monitor
	go test ./src/filter
	go test ./src/reporter

clean:
	rm -f senior-project-experiment c_fs_monitor/znotify \
		c_fs_monitor/test_fs_monitor

.PHONY: deps clean all test
