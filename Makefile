CC=gcc

all: senior-project-experiment c_fs_monitor/inotify

senior-project-experiment: main.go src/
	go build

c_fs_monitor/test_fs_monitor: c_fs_monitor/test_fs_monitor.c
	${CC} -o c_fs_monitor/test_fs_monitor c_fs_monitor/test_fs_monitor.c

c_fs_monitor/inotify: c_fs_monitor/inotify.c
	${CC} -o c_fs_monitor/inotify c_fs_monitor/inotify.c

test: c_fs_monitor/test_fs_monitor
	go test ./src/configurator
	go test ./src/monitor
	go test ./src/reporter
	go test ./src/filter

clean:
	rm -f senior-project-experiment c_fs_monitor/inotify \
		c_fs_monitor/test_fs_monitor
