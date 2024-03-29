# This regression file covers mistakes made in the Computer Go Ladder
# game between GNU Go 2.7.168 (white) and NeuroGo II (black) on
# September 9, 2000.

# Reset applicable counters
reset_connection_node_counter
reset_owl_node_counter
reset_reading_node_counter
reset_trymove_counter

loadsgf games/FSF-neurogo.sgf 20
1 gg_genmove white
#? [Q3]

loadsgf games/FSF-neurogo.sgf 22
2 gg_genmove white
#? [R3]

loadsgf games/FSF-neurogo.sgf 34
3 gg_genmove white
#? [H3|K3|K4]

# Tenuki is acceptable. In the local area the game move is bad.
loadsgf games/FSF-neurogo.sgf 40
4 gg_genmove white
#? [D6]*

loadsgf games/FSF-neurogo.sgf 50
5 gg_genmove white
#? [!F8]

# Tenuki is perhaps acceptable.
loadsgf games/FSF-neurogo.sgf 86
6 gg_genmove white
#? [C14]*

loadsgf games/FSF-neurogo.sgf 90
7 gg_genmove white
#? [O2]*

loadsgf games/FSF-neurogo.sgf 92
8 gg_genmove white
#? [O2]

loadsgf games/FSF-neurogo.sgf 124
9 gg_genmove white
#? [A18]*

loadsgf games/FSF-neurogo.sgf 136
10 gg_genmove white
#? [!A3]

# Semeai problem.
loadsgf games/FSF-neurogo.sgf 148
11 gg_genmove white
#? [N5]*

loadsgf games/FSF-neurogo.sgf 152
12 gg_genmove white
#? [B10]

loadsgf games/FSF-neurogo.sgf 176
# This may be hard to fix at this point but since we
# can't win the ko we shouldn't start it.
13 gg_genmove white
#? [!P7]

loadsgf games/FSF-neurogo.sgf 178
14 gg_genmove white
#? [Q4]*

# White A16 is about three points reverse sente. G19 is two points
# double sente, but black can get in A16 in sente before answering
# G19. We accept both moves as being correct.
loadsgf games/FSF-neurogo.sgf 230
15 gg_genmove white
#? [A16|G19]

# It doesn't look like black can live, so this would only be a
# reinforcing move. It may not be worth regressing over.
loadsgf games/FSF-neurogo.sgf 276
16 gg_genmove white
#? [H17|K17|H15|L16]

loadsgf games/FSF-neurogo.sgf 286
# K19 makes ko. H18 kills right out.
17 gg_genmove white
#? [H18]

loadsgf games/FSF-neurogo.sgf 288
18 gg_genmove white
#? [K19|H18|K16]

# Move 12
# M17 is an odd move. The local good shape and
# connection is at O16, but in order to really kill the black corner,
# a move at S16 is called for.
loadsgf games/FSF-neurogo.sgf 12
19 owl_attack Q17
#? [1 S16]
20 owl_defend Q17
#? [1 S16]
21 gg_genmove white
#? [S16]

# Move 18
# H16 looks strange. Locally better to make a wider extension towards
# the corner stone. Globally a move like R7 is more urgent,
# strengthening the lone lower right corner stone and keeping the
# black stones on the right side weak.
# (no actual test case yet)

# Move 34
# J3 is considered owl-dead and is therefore abandoned. This seems too
# pessismistic, however.
loadsgf games/FSF-neurogo.sgf 34
22 owl_attack J3
#? [1 K3]
23 owl_defend J3
#? [1 (K3|H3)]

# Move 88
# The C15 stones can be saved by a move at F16.
loadsgf games/FSF-neurogo.sgf 88
24 owl_attack C15
#? [1 (F16|E16|F14)]
25 owl_defend C15
#? [1 F16]

# Move 96
# The B16 stones can no longer be saved. Better to make connection at
# O2.
loadsgf games/FSF-neurogo.sgf 96
26 owl_attack B16
#? [1 (PASS|B17)]
27 owl_defend B16
#? [0]
28 gg_genmove white
#? [O2]

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
