# ping

non-Raw socket means you can just determine Transport Layer Payload. i.e it is the OS' task to create the Transport, Network, and Data Link layer headers.

Raw socket means you can determine every section of a packet, be it header or payload. Please note that raw socket is a general word. I categorize raw socket into: Network Socket and Data-Link Socket (or alternativly L3 Socket and L2 Socket).

- [Leak bug getaddrinfo](https://bugs.kde.org/show_bug.cgi?id=448991)
    - [stackoverflow](https://stackoverflow.com/questions/77642568/valgrind-showing-still-reachable-memory-leak-with-getaddrinfo)
    - [another stackoverflow](https://stackoverflow.com/questions/73976258/memory-leaks-observed-in-getgrnam-and-getgrnam-r-in-linux)

```sh
# show leak bug
sudo valgrind --leak-check=full  --show-leak-kinds=all ./ft_ping google.com
# suppression of leak bug
sudo valgrind  --suppressions=dlopen.supp ./ft_ping google.com
```

### TODO

- [ ] gerer le ping 127.0.0.1
    - verifier que le type de la reponse ne soit pas 8, sinon quand je loop back je recois mon propre ping. Si c'est 8 je laisse tomber le recv et je passe au suivant
- [ ] gerer les codes erreur pour afficher les bonnes erreurs
    - fonction `print_error_code();`  ping.c -> l.150
    - continuer la loop, mais faire une affichage particulier
    - `From 10.34.254.254 icmp_seq=15 Time to live exceeded`
    - [docu icmp](https://sites.uclouvain.be/SystInfo/usr/include/netinet/ip_icmp.h.html)
- [ ] modifier le code pour passer les variables depuis la variable global et pas les parametre 

https://www.geeksforgeeks.org/ping-in-c/