#!/usr/bin/env bash
# Script derived from
# https://community.arm.com/arm-community-blogs/b/internet-of-things-blog/posts/build-a-matter-home-automation-service-using-raspberry-pi-arm-virtual-hardware-and-python

[ "$UID" -eq 0 ] || exec sudo "$0" "$@"

apt-get update
apt-get install git gcc g++ python3 pkg-config libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev python3-pip unzip libgirepository1.0-dev libcairo2-dev libreadline-dev -y


git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

git checkout db49235c39635582ea522929f9905af03e3114c7


./scripts/checkout_submodules.py --shallow --platform linux


./scripts/build/gn_bootstrap.sh

source scripts/activate.sh


./scripts/build_python.sh -d true -i separate


source ./out/python_env/bin/activate