Installation
------------
On OS X with homebrew you'll need VirtualBox and vagrant:

First, initialize and start the vm:
```
$ vagrant up --provision
```

Next, ssh to the vm and enter the senior-project-experiment directory:
```
$ vagrant ssh
$ cd senior-project-experiment
```

To start the honeypot, run:
```
$ docker-compose up -d mysql
$ docker-compose up -d wordpress
```

Next visit http://localhost:8000 in your web browser and finish configuring
WordPress.

Monitoring
----------

Configure `ip netns`:
```
$ sudo mkdir -p /var/run/netns/
$ sudo ln -sf /proc/`docker inspect -f '{{ .State.Pid }}' seniorprojectexperiment_wordpress_1`/ns/net /var/run/netns/wordpress
```

```
$ sudo ip netns exec wordpress tcpdump > dump
```
