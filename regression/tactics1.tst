# Reset applicable counters
reset_connection_node_counter
reset_owl_node_counter
reset_reading_node_counter
reset_trymove_counter

# Apparently, T9 gets overvalued because the effect on S7 and T6 is
# counted twice (territorially and strategically)
loadsgf games/tactics05.sgf
10 gg_genmove white
#? [!T9]

loadsgf games/tactics06.sgf
20 owl_defend C19
#? [1 A17]*

# J1 is worth 6 points
loadsgf games/seki03.sgf
30 gg_genmove white
#? [J1]*


# attack_either and defend_both tests could go in a separate test suite,
# once there is more of them

loadsgf games/nngs/Lazarus-gnugo-3.1.34-200204280120.sgf 32
trymove black D12
trymove white C13
trymove white C12
trymove black C11
101 attack_either C11 D12
#? [1]*
popgo
popgo
popgo
popgo

# This is subtle. White N3 works as defense to L4.
loadsgf games/mertin13x13/katsunari-gnugo2.W+4.sgf 126
trymove black L4
102 defend_both N7 K4
#? [0]*
popgo

loadsgf games/nngs/Lazarus-gnugo-3.2-200205011927.sgf 86
trymove black G4
trymove white G3
103 attack_either E5 G3
#? [1]
104 attack_either G3 E5
#? [1]*
popgo
popgo

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
