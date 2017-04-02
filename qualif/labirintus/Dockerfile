FROM ubuntu:16.04

RUN apt-get update && apt-get --yes install software-properties-common python-software-properties
RUN add-apt-repository --yes ppa:ubuntu-toolchain-r/test
RUN apt-get update && apt-get --yes install build-essential g++-6 cmake openssh-server sudo
RUN sed -i 's|session    required     pam_loginuid.so|session    optional     pam_loginuid.so|g' /etc/pam.d/sshd
RUN mkdir -p /var/run/sshd
RUN useradd -m -d /home/lab -s /bin/bash lab && usermod -aG sudo lab
RUN echo "lab:lab" | chpasswd

RUN apt-get update && apt-get --yes build-dep libsfml-dev
RUN apt-get update && apt-get --yes install unzip libxrandr-dev git gdb
RUN git clone https://github.com/SFML/SFML.git
RUN cd SFML && wget https://gitlab.peach-bun.com/pinion/SFML/commit/3383b4a472f0bd16a8161fb8760cd3e6333f1782.patch -O sfml.patch && git apply sfml.patch
RUN cd SFML && ls && cmake -DCMAKE_BUILD_TYPE=Debug . && make -j && make install

RUN mkdir labirintus
COPY . labirintus/
ENV CC=gcc-6
ENV CXX=g++-6
RUN cd labirintus && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D"]
