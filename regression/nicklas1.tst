# =============================
# Private test set nicklas1.tst
# All of these tests passes with version 2.7.222.
# =============================

# Reset applicable counters
reset_connection_node_counter
reset_owl_node_counter
reset_reading_node_counter
reset_trymove_counter

loadsgf games/nicklas/nicklas1.sgf 9
101 gg_genmove black
#? [!D9]

loadsgf games/nicklas/nicklas2.sgf 5
201 gg_genmove black
#? [!G8|D8|F8]

loadsgf games/nicklas/nicklas2.sgf 9
202 gg_genmove black
#? [!H8]

#CATEGORY=OWL_TUNING
#DESCRIPTION=Combination threat to live / connect.
#SEVERITY=4
loadsgf games/nicklas/nicklas2.sgf 23
203 gg_genmove black
#? [J5]*

##tm - if B J8, W J5 kills bigger B corner.  
##So, there's no really effective move here.
## GNU Go 3.1.16 gets J5, which is prob. best.
#loadsgf games/nicklas/nicklas2.sgf 39
#204 gg_genmove black
##? [F8]*

loadsgf games/nicklas/nicklas2.sgf 55
205 dragon_status G1
#? [dead]

loadsgf games/nicklas/nicklas3.sgf 49
301 gg_genmove black
#? [H3|H6]

loadsgf games/nicklas/nicklas5.sgf 23
501 gg_genmove black
#? [G7]

loadsgf games/nicklas/nicklas5.sgf 27
502 gg_genmove black
#? [G5]

loadsgf games/nicklas/nicklas6.sgf 31
# semeai problem
603 gg_genmove black
#? [C5]

loadsgf games/nicklas/nicklas6.sgf 41
604 gg_genmove black
#? [A7|J9]

loadsgf games/nicklas/nicklas8.sgf 96
801 gg_genmove black
#? [P16]

loadsgf games/nicklas/nicklas8.sgf 162
806 gg_genmove black
#? [!G2]

loadsgf games/nicklas/nicklas8.sgf 164
807 gg_genmove black
#? [!H1]

loadsgf games/nicklas/nicklas8.sgf 180
808 gg_genmove black
#? [!L2]

loadsgf games/nicklas/nicklas8.sgf 224
810 gg_genmove black
#? [!C19]

# Moved K15 to K14. /gf ; Added back K14 - both work.
#SEVERITY=7
#CATEGORY=CONNECTION_TUNING
#DESCRIPTION=Is GnuGo misreading the cut?
loadsgf games/nicklas/nicklas8.sgf 240
811 gg_genmove black
#? [K14|K15]*

loadsgf games/nicklas/nicklas9.sgf 28
901 gg_genmove black
#? [!PASS]

loadsgf games/nicklas/nicklas10.sgf 30
1002 gg_genmove black
#? [H8]

loadsgf games/nicklas/nicklas10.sgf 56
1003 gg_genmove black
#? [!J6]

loadsgf games/nicklas/nicklas10.sgf 64
1004 gg_genmove black
#? [!J9]

loadsgf games/nicklas/nicklas10.sgf 66
1005 gg_genmove black
#? [!H7]

loadsgf games/nicklas/nicklas10.sgf 68
1006 gg_genmove black
#? [!J7]

loadsgf games/nicklas/nicklas11.sgf 16
1101 gg_genmove black
#? [!D11]

loadsgf games/nicklas/nicklas11.sgf 174
1104 gg_genmove black
#? [!A15]

loadsgf games/nicklas/nicklas11.sgf 180
1105 gg_genmove black
#? [!H14]

# Q6 seems largest but we may want to accept some more moves. The
# original formulation !F6 isn't very good since a one point reverse
# sente move isn't all that bad in the position.
loadsgf games/nicklas/nicklas11.sgf 208
1106 gg_genmove black
#? [Q6]*

#CATEGORY=WASTED_MOVE
#DESCRIPTION=GnuGo 3.1.8 gets N2 on next move;  R3 benign ko threat loss.
#SEVERITY=2
loadsgf games/nicklas/nicklas11.sgf 242
1107 gg_genmove black
#? [N2]*

# Q18 is the proper move to finally settle the position, but not
# really necessary. Let's just make sure T18 isn't played. /gf
loadsgf games/nicklas/nicklas11.sgf 258
1108 gg_genmove black
#? [!T18]

loadsgf games/nicklas/nicklas11.sgf 260
1109 gg_genmove black
#? [!T13]

# O18 is the proper move to finally settle the position, but not
# really necessary. Let's just make sure T15 isn't played. /gf
loadsgf games/nicklas/nicklas11.sgf 268
1110 gg_genmove black
#? [!T15]

loadsgf games/nicklas/nicklas11.sgf 270
1111 gg_genmove black
#? [N18]

# Q19 is not a fail, it does work as well. /pp
loadsgf games/nicklas/nicklas11.sgf 274
1112 gg_genmove black
#? [N19|Q19]

loadsgf games/nicklas/nicklas12.sgf 103
1206 gg_genmove black
#? [C15]

#CATEGORY=PATTERN_TUNING
#DESCRIPTION=Needless 2 point loss for B.
#SEVERITY=7
loadsgf games/nicklas/nicklas12.sgf 113
1207 gg_genmove black
#? [C17]*

loadsgf games/nicklas/nicklas12.sgf 117
1208 gg_genmove black
#? [E19]

loadsgf games/nicklas/nicklas12.sgf 121
1209 gg_genmove black
#? [!L19]

loadsgf games/nicklas/nicklas12.sgf 123
1210 dragon_status D18
#? [dead]

#CATEGORY=SEMEAI
#DESCRIPTION=
#SEVERITY=
loadsgf games/nicklas/nicklas12.sgf 203
1213 gg_genmove black
#? [N4]

# Added A6 among the correct moves. /gf (2.7.203)
loadsgf games/nicklas/nicklas12.sgf 215
1214 gg_genmove black
#? [A1|A6]

# H3 doesn't seem critical. /gf
loadsgf games/nicklas/nicklas12.sgf 259
1216 gg_genmove black
#? [H3]*

loadsgf games/nicklas/nicklas13.sgf 57
1302 gg_genmove black
#? [!G8]

loadsgf games/nicklas/nicklas13.sgf 58
1303 dragon_status G8
#? [dead]

loadsgf games/nicklas/nicklas13.sgf 73
1304 gg_genmove black
#? [!J8]

loadsgf games/nicklas/nicklas14.sgf 51
1404 gg_genmove black
#? [J7]

loadsgf games/nicklas/nicklas14.sgf 55
1405 gg_genmove black
#? [B7]

# Semeai problem
loadsgf games/nicklas/nicklas14.sgf 57
1406 gg_genmove black
#? [B6]

loadsgf games/nicklas/nicklas14.sgf 61
1408 dragon_status A8
#? [!dead]

loadsgf games/nicklas/nicklas15.sgf 196
1502 gg_genmove black
#? [D14]

loadsgf games/nicklas/nicklas15.sgf 208
1504 gg_genmove black
#? [!F16]

loadsgf games/nicklas/nicklas15.sgf 212
1505 gg_genmove black
#? [!O11]

loadsgf games/nicklas/nicklas15.sgf 224
1506 gg_genmove black
#? [S4]

loadsgf games/nicklas/nicklas15.sgf 228
1507 gg_genmove black
#? [!T1]

loadsgf games/nicklas/nicklas15.sgf 232
1508 gg_genmove black
#? [!F15]

loadsgf games/nicklas/nicklas15.sgf 258
1509 gg_genmove black
#? [!L1]

loadsgf games/nicklas/nicklas15.sgf 284
1510 gg_genmove black
#? [C12]

loadsgf games/nicklas/nicklas15.sgf 296
1511 gg_genmove black
#? [!J1]

loadsgf games/nicklas/nicklas15.sgf 298
1512 gg_genmove black
#? [!A1]

loadsgf games/nicklas/nicklas16.sgf 22
1601 gg_genmove black
#? [!D9]

loadsgf games/nicklas/nicklas16.sgf 28
1602 gg_genmove black
#? [!H8]

# Missed connection. See also connection:26,27
loadsgf games/nicklas/nicklas16.sgf 38
1603 gg_genmove black
#? [B4]

loadsgf games/nicklas/nicklas16.sgf 62
1604 gg_genmove black
#? [J2]

loadsgf games/nicklas/nicklas16.sgf 64
1605 gg_genmove black
#? [J1]

#CATEGORY=OWL_TUNING
#DESCRIPTION=No reason to give W a chance for ko here.
#SEVERITY=6
#The problem with D5 is the same as in nicklas2:1802
loadsgf games/nicklas/nicklas18.sgf 15
1801 gg_genmove black
#? [D2]

loadsgf games/nicklas/nicklas19.sgf 34
1901 gg_genmove black
#? [B7]

loadsgf games/nicklas/nicklas19.sgf 46
1902 gg_genmove black
#? [!H4]

loadsgf games/nicklas/nicklas19.sgf 52
1903 gg_genmove black
#? [A9]

loadsgf games/nicklas/nicklas19.sgf 60
1904 gg_genmove black
#? [H9|J8]

loadsgf games/nicklas/nicklas20.sgf 37
2001 gg_genmove black
#? [G5]

loadsgf games/nicklas/nicklas20.sgf 41
2002 gg_genmove black
#? [!H9]

# A slow move
loadsgf games/nicklas/nicklas21.sgf 52
2101 gg_genmove black
#? [!E1]

loadsgf games/nicklas/nicklas23.sgf 26
2301 gg_genmove black
#? [G6|F7|H8]

loadsgf games/nicklas/nicklas23.sgf 40
2302 gg_genmove black
#? [F4]

loadsgf games/nicklas/nicklas23.sgf 52
2303 gg_genmove black
#? [G9]

loadsgf games/nicklas/nicklas23.sgf 60
2304 gg_genmove black
#? [PASS]

loadsgf games/nicklas/nicklas23.sgf 62
2305 gg_genmove black
#? [PASS]

############ End of Tests #################

# Report number of nodes visited by the tactical reading
10000 get_reading_node_counter
#? [0]&

# Report number of nodes visited by the owl code
10001 get_owl_node_counter
#? [0]&

# Report number of nodes visited by the life code
10002 get_connection_node_counter
#? [0]&

# Report number of trymoves/trykos visited by the test
10003 get_trymove_counter
#? [0]&
