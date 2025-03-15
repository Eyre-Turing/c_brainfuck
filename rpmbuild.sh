#!/bin/bash

cd "$(dirname "$0")" || exit 1

SOURCES_DIR="$(dirname "$(dirname "$(rpmbuild --eval '%buildroot')")")/SOURCES"

rm -rf "$SOURCES_DIR/bf-1.0"
mkdir -p "$SOURCES_DIR/bf-1.0"
cp -a * "$SOURCES_DIR/bf-1.0"
tar -zcf "$SOURCES_DIR/bf-1.0.tar.gz" -C "$SOURCES_DIR/" bf-1.0
rpmbuild -bb bf.spec


