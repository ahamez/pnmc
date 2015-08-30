FROM buildpack-deps:jessie-scm

RUN apt-get update\
&& apt-get install --no-install-recommends -y\
		   cmake\
 		   g++\
		   make\
&& rm -rf /var/lib/apt/lists/*

WORKDIR /tmp

RUN curl -L http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.gz/download\
|  tar -v -C /tmp -xz\
&& cd /tmp/boost_1_59_0\
&& ./bootstrap.sh --with-libraries=program_options,coroutine,context,system,thread,filesystem\
&& ./b2 -j 4 install\
&& cd /tmp && rm -rf boost_1_59_0

RUN git clone https://github.com/ahamez/libsdd.git\
&& git clone --recursive https://github.com/ahamez/pnmc.git

RUN cd pnmc\
&& mkdir build\
&& cd build\
&& cmake .. -DCMAKE_BUILD_TYPE=Release -DLIBSDD_PATH=/tmp/libsdd\
&& make -j 2\
&& make install\
&& cd /tmp\
&& rm -rf ./libsdd\
&& rm -rf ./pnmc
