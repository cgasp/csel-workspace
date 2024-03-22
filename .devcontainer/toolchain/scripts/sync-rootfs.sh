#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset
# set -o xtrace

rsync -crlpgoD -i \
  --exclude=/etc/ssh/** \
  --exclude=/etc/hostname \
  --exclude=/etc/group \
  --exclude=/etc/passwd \
  --exclude=/etc/shadow \
  --exclude=/etc/fstab \
  --exclude=THIS_IS_NOT_YOUR_ROOT_FILESYSTEM \
  /buildroot/output/target/ \
  /rootfs/