#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
# set -o xtrace

rsync -rlpgoD --itemize-changes \
  --ignore-existing \
  --exclude=THIS_IS_NOT_YOUR_ROOT_FILESYSTEM \
  /buildroot/output/target/ \
  /rootfs/
  
rsync -crlpgoD --itemize-changes \
  --exclude=/etc/ssh/** \
  --exclude=/etc/hostname \
  --exclude=/etc/group \
  --exclude=/etc/passwd \
  --exclude=/etc/shadow \
  --exclude=/etc/fstab \
  --exclude=THIS_IS_NOT_YOUR_ROOT_FILESYSTEM \
  /buildroot/output/target/ \
  /rootfs/