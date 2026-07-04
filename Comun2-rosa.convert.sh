tshark -r Comun2-rosa.pcapng \
-t ad \
-T fields \
-e frame.time \
-e ip.src \
-e tcp.payload > Comun2-rosa.cat253-ALL.txt

#-Y "tcp.payload[0:1] == fd" \
