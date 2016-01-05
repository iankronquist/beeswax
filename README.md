# Installation
This honeypot only runs on Linux. Luckily for OS X users, there is a
Vagrantfile in the root of the repo to make starting development on OS X easy.

## Dependencies
You will need this software installed to run the honeypot. If you are using the
Vagrantfile this software will be installed as part of provisioning.
* Docker
* Docker Compose
* tcpdump
* Golang (build)

## Using Vagrant
On OS X you'll need to install VirtualBox and vagrant.

First, initialize and start the vm:
```
$ vagrant up --provision
```

Next, ssh to the vm and enter the senior-project-experiment directory:
```
$ vagrant ssh
$ cd senior-project-experiment
```

# Building
Run:
```
$ go build
```

Next visit http://localhost:8000 in your web browser and finish configuring
WordPress.

# Monitoring Notes

Configure `ip netns`:
```
$ sudo mkdir -p /var/run/netns/
$ sudo ln -sf /proc/`docker inspect -f '{{ .State.Pid }}' seniorprojectexperiment_wordpress_1`/ns/net /var/run/netns/wordpress
```

```
$ sudo ip netns exec wordpress tcpdump > dump
```

You can find all of the files the containers use under here:
/mnt/sda1/var/lib/docker/aufs/mnt
I found this out from reading `docker info`
