
[global]
version = 1.0


[http]
port = 3000

[zmq]
port = 3001

[pcap]
device = eth0
snaplen = 65535
# buffer_size in kilobytes (KB), default 2048 KBytes
buffer_size = 2048
filter = tcp
