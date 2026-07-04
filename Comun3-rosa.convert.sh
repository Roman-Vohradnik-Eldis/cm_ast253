tshark -r Comun3-rosa.pcapng \
-t ad \
-T fields \
-e frame.time \
-e ip.src \
-e tcp.payload > Comun3-rosa.cat253-ALL.txt

#-Y "tcp.payload[0:1] == fd" \
