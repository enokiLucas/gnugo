# Reset life node counter
reset_connection_node_counter
# Reset owl node counter
reset_owl_node_counter
# Reset reading node counter
reset_reading_node_counter

loadsgf games/niki.sgf 24
1 gg_genmove white
#? [E17]

#CATEGORY=MIDDLE_STRATEGY
#DESCRIPTION=Tough middle-game position
#SEVERITY=3
loadsgf games/niki.sgf 40
2 gg_genmove white
#? [E7]*

#CATEGORY=PATTERN_TUNING
#DESCRIPTION=Must connect 5 important stones, cutting B group.
#SEVERITY=10
loadsgf games/niki.sgf 68
3 gg_genmove white
#? [E12]

loadsgf games/niki.sgf 92
4 gg_genmove white
#? [Q8]

#S10 seems OK, much better than game move of S14. Added it -trevor
#S14 is thought to owl attack the corner. If this is correct the move
#looks okay. Otherwise we need to fix the owl reading. /gf
loadsgf games/niki.sgf 94
5 gg_genmove white
#? [S9|S10]*

loadsgf games/niki.sgf 106
6 gg_genmove white
#? [!S18]


#CATEGORY=PATTERN_TUNING
#DESCRIPTION=T4 is terrible; other moves than J12 maybe OK.
#SEVERITY=6
loadsgf games/niki.sgf 110
7 gg_genmove white
#? [J12]*

# H15 is less efficient but does secure life, which is most
# important. Added H15, G15, and F15. /gf
loadsgf games/niki.sgf 124
8 gg_genmove white
#? [G16|H15|G15|F15]

#gg_genmove bug requires reloading game.
loadsgf games/niki.sgf 124
9 gg_genmove black
#? [F15]

loadsgf games/niki.sgf 128
10 gg_genmove white
#? [S5]*


#CATEGORY=TACTICAL_READING
#DESCRIPTION=Very tough position
#SEVERITY=3
#Note: G16 is still very very big.  See niki.tst#8 & nikit.tst#9
#tm (3.1.16) added P6 - looks locally best, and GNU Go agrees.
loadsgf games/niki.sgf 158
11 gg_genmove white
#? [P6|M6|P7]*


#CATEGORY=OWL_TUNING
#DESCRIPTION=B15 does live.  Problem is w/ followup - see niki.tst#14
#SEVERITY=0
#
# Originally H12 was listed as correct here, but that vertex is
# occupied. F14 and G15 seem sufficient for life.
#
#Recommend adding B15 as acceptable to live.
loadsgf games/niki.sgf 212
12 gg_genmove white
#? [F14|G15]*

loadsgf games/niki.sgf 226
13 gg_genmove white
#? [F8]


#CATEGORY=OWL_TUNING
#DESCRIPTION=F16 lives because of B short of liberties
#SEVERITY=8
loadsgf games/niki.sgf 214
14 gg_genmove white
#? [F16]*

# Report number of nodes visited by the tactical reading
10000 get_reading_node_counter
#? [0]&

# Report number of nodes visited by the owl code
10001 get_owl_node_counter
#? [0]&

# Report number of nodes visited by the life code
10002 get_connection_node_counter
#? [0]&
