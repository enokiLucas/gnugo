
# M3 is overplay
loadsgf games/nngs/bconwil-gnugo-3.3.11-200211182313.sgf 18
10 gg_genmove black
#? [G3|F4]*

# This is of course too hard for GNU Go at the moment.
loadsgf games/nngs/bconwil-gnugo-3.3.11-200211182313.sgf 20
20 gg_genmove black
#? [G3|H3]*

loadsgf games/nngs/gnugo-3.3.12-RikiTiki-200212042341.sgf 36
30 gg_genmove white
#? [K3|J4|M5]*

# Definitely not T12 as in the game. /ab
loadsgf games/nngs/gnugo-3.3.12-RikiTiki-200212042341.sgf 50
40 gg_genmove white
#? [R10|S10|P11]*

loadsgf games/nngs/gnugo-3.3.12-RikiTiki-200212042341.sgf 52
50 restricted_genmove white S14 T13
#? [S14]

# The cut at R14 is not dangerous. /ab
loadsgf games/nngs/gnugo-3.3.12-RikiTiki-200212042341.sgf 110
60 gg_genmove white
#? [N8|M7|L7|M9|L9|N7|O9|N10]*

loadsgf games/nngs/gnugo-3.3.12-RikiTiki-200212042341.sgf 112
70 gg_genmove white
#? [N7|O9]*


