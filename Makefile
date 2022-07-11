
all: udpsend.so

udpsend.so: udpsend_udf.cc
	gcc -I  /opt/percona8/include/ -shared -o udpsend.so udpsend_udf.cc

