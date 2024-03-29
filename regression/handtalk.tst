
reset_owl_node_counter
reset_reading_node_counter
reset_trymove_counter

# No the right moment to start cosmic go at G7 as in the game.
loadsgf games/handtalk/handtalk12.sgf 17
1 gg_genmove black
#? [R12|R13|R14|R8|R7|R6]*

# Correct owl defense at P5 gets in 3.1.31 due to negative shape value
# -20 (!) by Pattern Shape13.
loadsgf games/handtalk/handtalk12.sgf 37
2 gg_genmove black
#? [P5|Q2|R2]*

# The sagari at F18 (very much liked by 3.1.26-32 influence code) is bad
# endgame here (as well as in many other situations). Also, defending N7
# seems more urgent.
loadsgf games/handtalk/handtalk12.sgf 55
3 gg_genmove black
#? [!F18]

# Have to try this ko!
loadsgf games/handtalk/handtalk12.sgf 91
4 gg_genmove black
#? [Q2]*

loadsgf games/handtalk/handtalk13.sgf 79
5 gg_genmove black
#? [!P15]

loadsgf games/handtalk/handtalk13.sgf 161
6 gg_genmove black
#? [!F16]

loadsgf games/handtalk/handtalk21.sgf 14
7 gg_genmove white
#? [R4]

loadsgf games/handtalk/handtalk21.sgf 24
8 gg_genmove white
#? [!G14]

loadsgf games/handtalk/handtalk21.sgf 46
9 gg_genmove white
#? [J17]*

# Other moves possible, but GNU Go should attack the D8 stone
loadsgf games/handtalk/handtalk23.sgf 26
10 gg_genmove white
#? [E9|F8|D8]

# B9 is quite bad here.
loadsgf games/handtalk/handtalk23.sgf 30
11 gg_genmove white
#? [C11|D10]*

# The balance of power between J11 and H8 is the issue at the moment.
# Maybe other moves possible.
loadsgf games/handtalk/handtalk23.sgf 38
12 gg_genmove white
#? [G9|K8|J9|K9]*

loadsgf games/handtalk/handtalk23.sgf 48
13 gg_genmove white
#? [!P14]

loadsgf games/handtalk/handtalk23.sgf 80
14 gg_genmove white
#? [K4]*

loadsgf games/handtalk/handtalk23.sgf 82
15 gg_genmove white
#? [J3]*

#CATEGORY=CONNECTION
loadsgf games/handtalk/handtalk23.sgf 96
16 gg_genmove white
#? [J18]*

loadsgf games/handtalk/handtalk23.sgf 162
17 gg_genmove white
#? [B4|B6]*

loadsgf games/handtalk/handtalk23.sgf 186
18 gg_genmove white
#? [S5]

# GNU Go played J8. Make sure it never forgets to do so!
loadsgf games/handtalk/handtalk1.sgf 69
19 gg_genmove black 
#? [J8]

loadsgf games/handtalk/handtalk2.sgf 79
20 gg_genmove black 
#? [N18]*

#CATEGORY=BLUNDER
# (Incorrectly rejected move)
loadsgf games/handtalk/handtalk2.sgf 137
21 gg_genmove black 
#? [R6]


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
