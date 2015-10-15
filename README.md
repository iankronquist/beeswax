Installation
------------
On OS X with homebrew you'll need VirtualBox and vagrant:

First, initialize and start the docker-machine vm:
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
