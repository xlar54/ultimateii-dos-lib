10 c$="128":b=842:t=208:ifpeek(1)and4thenc$="64":b=631:t=198
20 printchr$(147)chr$(14)"{wht}{dish}";:poke53280,0:poke53281,0
30 print"{CBM-D}{CBM-F}{CBM-D}{CBM-F}{CBM-I} {CBM-I}{CBM-I}{CBM-I}{CBM-D}{CBM-I}{CBM-F}{CBM-I} {CBM-D}{CBM-F} {CBM-I} {CBM-I}{CBM-I}{CBM-I}{CBM-D}{CBM-I}{CBM-I}{CBM-F}"
40 print"{rvon}{CBM-K}{rvof}{CBM-K}{rvon}{CBM-K}{rvof}{CBM-K}{rvon} {rvof}  {rvon} {rvof}  {rvon} {rvof} {rvon} {CBM-C} {rvof}{CBM-K}{rvon}{CBM-V}{CBM-I}{CBM-C}{rvof} {rvon} {rvof} {rvon}{CBM-K}{CBM-C}{rvof}{CBM-F}  {CBM-C}{rvon} {CBM-F}{CBM-D}{rvof}  DOS"
50 print"{rvon}{CBM-K}{rvof}{CBM-K}{rvon}{CBM-K}{rvof}{CBM-K}{rvon} {rvof}  {rvon} {rvof}  {rvon} {rvof} {rvon} {rvof}{CBM-C}{rvon}{CBM-K}{rvof}{CBM-K}{rvon} {CBM-I} {rvof} {rvon} {rvof} {rvon}{CBM-K}{rvof}{CBM-K}   {CBM-D}{rvon} {CBM-V}{CBM-C}{rvof}    Lib"
60 print" {rvon}{CBM-I}{CBM-I}{rvof} {rvon}{CBM-I}{CBM-I}{rvof}{CBM-V}{rvon}{CBM-I}{rvof} {CBM-C}{rvon}{CBM-I}{rvof}{CBM-V}{rvon}{CBM-I}{rvof} {CBM-C}{CBM-V}{rvon}{CBM-I}{rvof} {rvon}{CBM-I}{rvof} {rvon}{CBM-I}{rvof} {CBM-C}{rvon}{CBM-I}{CBM-I}{rvof}{CBM-V}"
70 print:print
80 ifc$="64"thenprint"          MAKE YOUR CHOICE"
90 ifc$="128"thenprint"  Programs for 80 column mode ONLY"
110 print:print"  {rvon} 1 {rvof}  Ultimate Term - BBS client"
120 print:print"  {rvon} 2 {rvof}  Ultimate Chat - IRC client"
130 geta$:ifa$<>"1"anda$<>"2"then130
140 ifa$="1"thenp$="u-term"
150 ifa$="2"thenp$="u-chat"
160 print:print"wait please...":print
170 print"load"chr$(34)p$c$chr$(34)","peek(186)
180 pokeb,145:pokeb+1,145:pokeb+2,145
190 pokeb+3,13:pokeb+4,82:pokeb+5,85:pokeb+6,78:pokeb+7,13
200 poket,8