#!/bin/bash

# Download and install ffmpeg on Ubuntu. not enabled all features support. the local compiler tool
# chain is llvm-clang-9.0

BASE_DIR=$(pwd)
mkdir ffmpeg_source ffmpeg_build dependencies

FF_SRC=$BASE_DIR/ffmpeg_source
FF_BIN=$BASE_DIR/ffmpeg_build
FF_DEP=$BASE_DIR/dependencies

# Prerequisite: add more if needed
printf "\nINFO: installing prerequisites\n\n"
sudo apt-get -y --force-yes install libfdk-aac-dev build-essential yasm nasm cmake

# libx264
printf "\nINFO: Fetching and building libx264\n\n"
LIB_X264=$FF_DEP/libx264
mkdir $LIB_X264
git clone https://code.videolan.org/videolan/x264.git $LIB_X264
cd $LIB_X264
./configure --enable-pic --enable-shared --disable-asm && make && sudo make install
printf "\nOK: libx264 installed.\n\n"
cd $BASE_DIR

printf "\nINFO: Fetching and building FFmpeg\n\n"
git clone https://github.com/FFmpeg/FFmpeg.git $FF_SRC
cd $FF_SRC
export PKG_CONFIG_PATH=$FF_BIN/lib/pkgconfig
export PATH="$FF_BIN/bin:$PATH"
./configure --enable-gpl --enable-libx264 --enable-shared --enable-libfdk-aac --enable-nonfree \
--prefix="$FF_BIN" --extra-cflags="-I$FF_BIN/include" --extra-ldflags="-L$FF_BIN/lib" \
--extra-libs="-lpthread -lm" --bindir="$FF_BIN/bin" && make && sudo make install
sudo ldconfig
printf "\nOK: FFmpeg installed.\n"
cd $BASE_DIR

# `ldd ffmpeg` check if missed some shared libs then install them
# find it's location and add location to /etc/ld.so.config the do `sudo ldconfig`
# eg. compiled ffmpeg libs locate at $FF_BIN/lib, may need to add the path to /etc/ld.so.config
# and update it to make ffmpeg bins knows where to link them
# also append $FF_BIN/bin to $PATH permanently, eg. in ~/.bashrc and execute `ffmpeg -version` in
# new shell. everything should be done now.