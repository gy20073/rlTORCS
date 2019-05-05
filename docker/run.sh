#!/usr/bin/env bash
docker run \
    -it \
    --volume="/data:/data2:rw" \
    --volume="/data1:/data1:rw" \
    --volume="/data2:/data2:rw" \
    --volume="/data4:/data4:rw" \
    --runtime=nvidia \
    -e NVIDIA_VISIBLE_DEVICES="0,1,2,3,4,5,6,7" \
    --net=host \
    gy20073/torcs \
    /bin/bash


