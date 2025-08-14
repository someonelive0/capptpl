
[global]
version = 1.0


[http]
port = 3000
# enable https, true or 1 means neable, default true
enable_ssl = true
crt_file = server.crt
key_file = server.key


[redis]
host = 127.0.0.1
port = 6379
passwd = 
# redis cmd is: BRPOP <list> <expired>, such as 'BRPOP queue1 queue2 queue3 10'
# so list is redis listnames as queue to BRPOP
# default expired seconds is 10
list = queue1 queue2 queue3 queue4

