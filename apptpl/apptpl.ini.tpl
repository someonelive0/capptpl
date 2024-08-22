
[global]
version = 1.0


[http]
port = 3000
# enable https, true or 1 means neable, default true
enable_ssl = true
crt_file = server.crt
key_file = server.key


[zmq]
port = 3001


[pcap]
device = eth0
snaplen = 65535
# buffer_size in kilobytes (KB), default 2048 KBytes
buffer_size = 2048
filter = tcp

[word_policy]
    word_file = words
