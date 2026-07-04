#!/bin/bash

#bin/Decode253

#PRVNI ZPRAVA z meho PC do 192.168.5.103
bin/Decode253 fd 00 0d 8b 20 64 36 00 03 02 58 d8 0e
#^ 253 cat
#   ^ 13 bytes length
#         ^ FSPEC I253/010 , I253/040 , I253/070 , I253/080
#               ^ I253/010   SAC=100  SIC=54
#                     ^ I253/040 
#               I253/070
#               I253/080
#               
#DRUHA ZPRAVA - ODPOVED
bin/Decode253 fd 00 10 ab 20 31 de 01 64 36 00 03 03 58 c1 44
#                                       TIME
##                                    03-conect response
#               31 de = sac=49 sic=222
#
#
#
#
#Nejaka delsi zprava s nazvama programu/patternu, ale jsou i delsi zpravy
#                                         TIME?
bin/Decode253 fd 00 de ab 32 31 de 01 64 36 00 03 0a   58 c1 4b 04 15 01 10 ca 01 02 06 00 35 73 3a 4d 31 2e 32 30 30 48 7a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 35 73 3a 4d 32 2e 32 30 30 48 7a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 35 73 3a 4d 33 2e 32 30 30 48 7a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 35 73 3a 4d 43 2e 32 30 30 48 7a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 04 35 73 3a 4d 34 2e 32 30 30 48 7a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 05 35 73 3a 4d 53 4f 2e 41 43 2e 50 52 30 2e 32 30 30 48 7a 00 00 00 00 00 00 00 00 00 00 00 00 00


