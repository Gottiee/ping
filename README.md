# ping

ping uses the ICMP protocol's mandatory ECHO_REQUEST datagram to elicit an ICMP ECHO_RESPONSE from a host or gateway. ECHO_REQUEST datagrams (''pings'') have an IP and ICMP header, followed by a struct timeval and then an arbitrary number of ''pad'' bytes used to fill out the packet. 

## usage

```sh
Usage: ping [OPTION...] HOST ...
Send ICMP ECHO_REQUEST packets to network hosts.

Options:
  -v            verbose output
  -?            give this help list
  -q            quiet output
  -c<number>    stop after sending <number> packets
  -ttl<number>  specify <number> as time-to-live
  -W<number>    wait <number> seconds for response
  -i<number>    wait <number> seconds between sending each packet
```


- [Leak bug getaddrinfo](https://bugs.kde.org/show_bug.cgi?id=448991)
    - [stackoverflow](https://stackoverflow.com/questions/77642568/valgrind-showing-still-reachable-memory-leak-with-getaddrinfo)
    - [another stackoverflow](https://stackoverflow.com/questions/73976258/memory-leaks-observed-in-getgrnam-and-getgrnam-r-in-linux)

```sh
# show leak bug
sudo valgrind --leak-check=full  --show-leak-kinds=all ./ft_ping google.com
# suppression of leak bug
sudo valgrind  --suppressions=dlopen.supp ./ft_ping google.com
```

## Shema

Crafting ICMP packet :

![icmppacket](/img/icmp.png)

### Manage error

Identify error icmp packet:

![icmp error](/img/icmp_problem.png)

## Docu

https://www.geeksforgeeks.org/ping-in-c/