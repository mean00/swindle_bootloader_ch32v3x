#!/usr/bin/bash
export PATH=/home/${USER}/.cargo/bin:/home/${USER}/.local/bin:$PATH
export ROOT=$PWD

runbuild() {
  local cur_dir=$PWD
  local folder_name="build${1}"
  shift
  rm -Rf $cur_dir/$folder_name
  mkdir $cur_dir/$folder_name
  cd $folder_name
  cmake ${COMMON} $@ ..
  make -j 4
  cd $cur_dir
}

runbuild build -DUSE_CLANG=True
