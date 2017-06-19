FROM ubuntu
RUN apt-get update && \
	apt-get install -y \
		build-essential \
        cmake \
		git \
		libssl-dev \
        libcurl4-openssl-dev \
		libboost-system-dev \
		libboost-thread-dev

RUN mkdir -p /usr/code && \
	cd /usr/code && \
	git clone https://github.com/arangodb/velocypack.git && \
	cd velocypack && \
	mkdir -p build && \
	cd build && \
	cmake .. && \
	make install && \
	cd / && rm -Rf /usr/code 

ENTRYPOINT "/bin/bash"
