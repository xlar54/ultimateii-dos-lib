5 printchr$(147)chr$(142)"{wht}{dish}";:poke53280,0:poke53281,0
10 print"{CBM-D}{CBM-F}{CBM-D}{CBM-F}{CBM-I} {CBM-I}{CBM-I}{CBM-I}{CBM-D}{CBM-I}{CBM-F}{CBM-I} {CBM-D}{CBM-F} {CBM-I} {CBM-I}{CBM-I}{CBM-I}{CBM-D}{CBM-I}{CBM-I}{CBM-F}"
20 print"{rvon}{CBM-K}{rvof}{CBM-K}{rvon}{CBM-K}{rvof}{CBM-K}{rvon} {rvof}  {rvon} {rvof}  {rvon} {rvof} {rvon} {CBM-C} {rvof}{CBM-K}{rvon}{CBM-V}{CBM-I}{CBM-C}{rvof} {rvon} {rvof} {rvon}{CBM-K}{CBM-C}{rvof}{CBM-F}  {CBM-C}{rvon} {CBM-F}{CBM-D}{rvof}  dos"
30 print"{rvon}{CBM-K}{rvof}{CBM-K}{rvon}{CBM-K}{rvof}{CBM-K}{rvon} {rvof}  {rvon} {rvof}  {rvon} {rvof} {rvon} {rvof}{CBM-C}{rvon}{CBM-K}{rvof}{CBM-K}{rvon} {CBM-I} {rvof} {rvon} {rvof} {rvon}{CBM-K}{rvof}{CBM-K}   {CBM-D}{rvon} {CBM-V}{CBM-C}{rvof}    lib"
40 print" {rvon}{CBM-I}{CBM-I}{rvof} {rvon}{CBM-I}{CBM-I}{rvof}{CBM-V}{rvon}{CBM-I}{rvof} {CBM-C}{rvon}{CBM-I}{rvof}{CBM-V}{rvon}{CBM-I}{rvof} {CBM-C}{CBM-V}{rvon}{CBM-I}{rvof} {rvon}{CBM-I}{rvof} {rvon}{CBM-I}{rvof} {CBM-C}{rvon}{CBM-I}{CBM-I}{rvof}{CBM-V}"
50 print:print
60 print"  make your choice:"
70 print:print"  {rvon} 1 {rvof}  ultimate term - bbs client"
80 print:print"  {rvon} 2 {rvof}  ultimate chat - irc client"
90 geta$:ifa$<>"1"anda$<>"2"then90
100 ifa$="1"thenp$="u-term"
110 ifa$="2"thenp$="u-chat"
120 print:print"wait please...":print
130 a$="128":b=842:t=208:ifpeek(1)and4thena$="64":b=631:t=198
140 print"load"chr$(34)p$a$chr$(34)","peek(186)
150 pokeb,145:pokeb+1,145:pokeb+2,145
160 pokeb+3,13:pokeb+4,82:pokeb+5,85:pokeb+6,78:pokeb+7,13:poket,8
