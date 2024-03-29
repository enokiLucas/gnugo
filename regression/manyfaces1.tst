# Reset applicable counters
reset_connection_node_counter
reset_owl_node_counter
reset_reading_node_counter
reset_trymove_counter

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 9
10 gg_genmove black
#? [C17|H15|D9]

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 15
20 gg_genmove black
#? [F12|G14|G13|C17|C12|E13]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 25
30 gg_genmove black
#? [C14]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 41
33 gg_genmove black
#? [S17|R3]

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 47
36 gg_genmove black
#? [P16]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 59
40 gg_genmove black
#? [P15]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 65
50 owl_attack K17
#? [1 .*]*

# W:P19 threatens the corner but still N18 is important
loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 79
60 gg_genmove black
#? [N18|G18]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 83
70 gg_genmove black
#? [M10|L9|L8|K7]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 97
80 gg_genmove black
#? [J17|N18|D18]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 99
90 gg_genmove black
#? [!A10]

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 101
100 gg_genmove black
#? [!B13|A13]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 191
150 gg_genmove black
#? [N5|O6]*

loadsgf games/nngs/ManyFaces3-gnugo-3.3.11-200211071935.sgf 201
160 gg_genmove black
#? [O5]*



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
