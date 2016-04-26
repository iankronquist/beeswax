#!/bin/bash

set -e
set -x

SERVER_URL=$1
DEPLOY_KEY=$2

export GOPATH=/opt/beeswax_gopath
INSTALL_PATH=$GOPATH/src/
HONEYPOT_NAME=senior-project-experiment
MHN_PATH=/opt/mhn/


install_dependencies() {
	apt-get update
	apt-get -y install docker.io tcpdump gcc golang git supervisor python-pip
	pip install docker-compose
}

install_project() {
	if [[ ! -e $INSTALL_PATH ]]; then
		mkdir -p $INSTALL_PATH
		git clone http://github.com/iankronquist/senior-project-experiment.git $INSTALL_PATH/$HONEYPOT_NAME
	fi
	cd $INSTALL_PATH/$HONEYPOT_NAME
	make all
}

# The docker group must already exist
# The project must already be cloned
make_user() {
	groupadd docker
	useradd -d /home/beeswax -s /bin/bash -m beeswax -g users -G docker
	chown -R beeswax $INSTALL_PATH
}

make_configs() {
	source $MHN_PATH/scripts/registration.sh $SERVER_URL $DEPLOY_KEY "beeswax"
	cat  > $INSTALL_PATH/$HONEYPOT_NAME/honeypot_config.json <<EOF
{
  "monitor process name": "c_fs_monitor/znotify",
  "docker compose name": "docker-compose",
  "container names": ["mysql", "wordpress"],
	"mhn host": "$HPF_HOST",
	"mhn port": $HPF_PORT,
	"mhn identifier": "$HPF_IDENT",
	"mhn authorization": "$HPF_SECRET"
}
EOF
	cp $INSTALL_PATH/$HONEYPOT_NAME/beeswaf_supervisor.conf /etc/supervisor/conf.d/beeswax.conf
}

# The project must already be cloned
enable_services() {
	# Ensure docker is enabled and started
	systemctl enable docker
	systemctl start docker

	supervisorctl update
}


install_dependencies
install_project
make_user
make_configs
enable_services
