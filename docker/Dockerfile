FROM nvidia/cuda:8.0-devel-ubuntu16.04

RUN apt-get -yqq update
RUN apt-get -yqq install wget iputils-ping htop software-properties-common python-software-properties git vim-gui-common vim-runtime ffmpeg tmux python-pip

RUN pip install tensorflow-gpu

# torch
WORKDIR /root/
RUN git clone https://github.com/torch/distro.git ~/torch --recursive
WORKDIR /root/torch
RUN sed -i -e 's/sudo//g' install-deps
RUN bash install-deps
# this fix the half precison compile error
ENV TORCH_NVCC_FLAGS "-D__CUDA_NO_HALF_OPERATORS__"
WORKDIR /root/torch
RUN bash install.sh

# fix the original torch could only use cudnn v5 issue
WORKDIR /root/
RUN bash /root/torch/install/bin/torch-activate && git clone https://github.com/soumith/cudnn.torch.git -b R7 && cd cudnn.torch && /root/torch/install/bin/luarocks make cudnn-scm-1.rockspec

RUN apt-get update
RUN apt-get -yqq install python-tk

RUN apt-get install -yqq libopencv-dev
RUN apt-get install -yqq openssh-client

#RUN pip2 install scipy
#RUN pip2 install h5py imgaug pygame

# install xvfb
RUN apt-get install -yqq xvfb
RUN apt-get install -yqq mesa-utils

# install lunatic python
WORKDIR /root/
RUN git clone https://github.com/gy20073/lunatic-python.git /root/lunatic_python --recursive
WORKDIR /root/lunatic_python
RUN python setup.py install

# install torcs env
WORKDIR /root/
RUN git clone https://github.com/gy20073/rlTORCS.git --recursive
WORKDIR /root/rlTORCS/torcs-1.3.6
RUN make clean
WORKDIR /root/rlTORCS

RUN echo "deb-src http://archive.ubuntu.com/ubuntu/ xenial main restricted" >> /etc/apt/sources.list
RUN echo "deb-src http://archive.ubuntu.com/ubuntu/ xenial-updates main restricted" >> /etc/apt/sources.list
RUN echo "deb-src http://archive.ubuntu.com/ubuntu/ xenial universe" >> /etc/apt/sources.list
RUN echo "deb-src http://archive.ubuntu.com/ubuntu/ xenial-updates universe" >> /etc/apt/sources.list
RUN apt-get update
WORKDIR /root/rlTORCS
RUN ./install_nosudo.sh
WORKDIR /root/
# change the default resolution
RUN torcs
RUN sed -i 's/640/160/g' /root/.torcs/config/screen.xml
RUN sed -i 's/480/120/g' /root/.torcs/config/screen.xml

RUN wget https://repo.anaconda.com/archive/Anaconda3-2019.03-Linux-x86_64.sh
RUN bash ./Anaconda3-2019.03-Linux-x86_64.sh -b -p /root/anaconda/
RUN pip install tensorflow-gpu==1.4.0
RUN pip install gym==0.9.2
RUN pip install opencv-python
