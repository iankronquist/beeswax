Installation
------------
On OS X with homebrew you'll need VirtualBox, Docker, docker-machine, and
docker-compose:
```
$  brew install docker docker-machine docker-compose
```

First, initialize and start the docker-machine vm:
```
$ docker-machine create --driver virtualbox dev
$ eval "$(docker-machine env dev)"
```

To start the honeypot, run:
```
$ docker-compose up -d mysql
$ docker-compose up -d wordpress
```

Now, find the IP address the vm is using, and open that URL with
port 8000 in your browser:
```
$ docker-machine ip dev
$ open http://$(docker-machine ip dev):8000
```

http://serverfault.com/questions/615854/counting-bandwidth-from-a-docker-container
http://stackoverflow.com/questions/11061512/log-all-traffic-on-https-with-iptables-rules
