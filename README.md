# ping

non-Raw socket means you can just determine Transport Layer Payload. i.e it is the OS' task to create the Transport, Network, and Data Link layer headers.

Raw socket means you can determine every section of a packet, be it header or payload. Please note that raw socket is a general word. I categorize raw socket into: Network Socket and Data-Link Socket (or alternativly L3 Socket and L2 Socket).