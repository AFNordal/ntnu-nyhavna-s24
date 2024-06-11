FROM debian:12
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /root

RUN apt clean && apt update && apt upgrade
RUN apt install -y  build-essential cmake git

# pico
RUN apt install -y gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib

# c++ libraries
WORKDIR /include
RUN git clone https://github.com/raspberrypi/pico-sdk.git --recurse-submodules
RUN git clone https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico.git --recurse-submodules

# python
ARG python=python3.11
RUN apt update && apt install -y ${python} python3-pip 
# RUN update-alternatives --install /usr/bin/python3 python3 /usr/bin/${python} 1
# RUN update-alternatives --config python3

RUN printf "%s\n" "alias pip=pip3" "alias pip3='DISPLAY= pip3'" "alias python=python3" > ~/.bash_aliases

RUN pip3 install --upgrade pip setuptools --break-system-packages
RUN pip3 install black --break-system-packages
RUN pip3 install websockets --break-system-packages
# RUN pip install numpy scipy
# RUN pip install plotly

# # js
# RUN apt install wget
# RUN apt install -y nodejs npm
# RUN npm install -g n && n stable

# gitconfig
COPY ssh-keys /root/.ssh

RUN git config --global core.fileMode false
# RUN git config --global core.autocrlf true
RUN git config --global --add safe.directory "*"
RUN git config --global user.email "nordalrasmus01@gmail.com"
RUN git config --global user.name "Rasmus Anker Fossen Nordal"

# remote display
WORKDIR /root

RUN echo "export DISPLAY=host.docker.internal:0.0" >> .bashrc
RUN echo "export LIBGL_ALWAYS_INDIRECT=1" >> .bashrc
RUN echo "export PICO_SDK_PATH=/include/pico-sdk" >> .bashrc