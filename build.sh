#!/bin/bash

set -uo pipefail

DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROGRAM="$0"
CLEAN=false
JOBS=$(nproc)

info() {
  echo -e "[$(date +'%Y-%m-%dT%H:%M:%S%z')][info] $*"
}

err() {
  echo -e "[$(date +'%Y-%m-%dT%H:%M:%S%z')][error] $*" >&2
}

usage() {
  cat <<EOF
Usage:  $PROGRAM [OPTIONS]
Options:
  -h,--help                  show this help
  -c,--clean                 clean build
  -j [N],--jobs [N]          specify the number of jobs

EOF
}

build() {
  local jobs="$1"
  local clean="$2"
  local dir="${DIR}/build"

  if [ "${clean}" == true ]; then
    rm -rf "${dir}"
  fi

  set -x

  if ! cmake -GNinja -B "${dir}"; then
    err "run cmake configuration failed."
    return 1
  fi

  if ! cmake --build "${dir}" -j "${jobs}" --verbose; then
    err "build source code faild."
    return 1
  fi
}

while (( "$#" )); do
  case "$1" in
    -h|-\?|--help) usage; exit 0 ;;
    -j|--jobs) JOBS="${2}"; shift 2 ;;
    -c|--clean) CLEAN=true; shift ;;
    --*=|-*) err "unsupported option $1"; exit 1 ;;
  esac
done

if ! build "${JOBS}" "${CLEAN}"; then
  exit 1
fi
