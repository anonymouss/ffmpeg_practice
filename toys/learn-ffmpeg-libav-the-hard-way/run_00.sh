#!/bin/bash

CUR=$(pwd)
PRJ=$(dirname $(dirname $CUR))
VID=$PRJ/media/v # video
AUD=$PRJ/media/a # audio
IMG=$PRJ/media/i # image

./out/00_hello_world $VID/small_bunny_1080p_60fps.mp4 $VID/invalid.url