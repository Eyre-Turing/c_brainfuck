#!/bin/bash

cd "$(dirname "$0")" || exit 1

mkdir -p ~/rpmbuild/SOURCES/bf-1.0
cp -a . ~/rpmbuild/SOURCES/bf-1.0
tar -zcf ~/rpmbuild/SOURCES/bf-1.0.tar.gz -C ~/rpmbuild/SOURCES/ bf-1.0
rpmbuild -bb bf.spec


