FROM ubuntu
RUN apt-get update && \
	apt-get install -y \
		build-essential \
        cmake \
		libssl-dev \
        libcurl4-openssl-dev \
		gdb

ENTRYPOINT "/bin/bash"