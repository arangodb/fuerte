FROM ubuntu
RUN apt-get update && \
	apt-get install -y \
		build-essential \
        cmake \
		libssl-dev \
        libcurl4-openssl-dev \
		libboost-system-dev \
		libboost-thread-dev \
		gdb

ENTRYPOINT "/bin/bash"