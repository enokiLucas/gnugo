# incident 72
# A15 and A16 are ko threats. These are of no use but don't hurt the
# position either. filllib shouldn't play these but they may get
# played as owl_threaten_attack moves. We include them here to avoid
# false alarms.
loadsgf games/incident72.sgf 255
1 gg_genmove black
#? [T15|T13|T10|T8|T5|T1|Q1|N1|A15|A16]
2 gg_genmove white
#? [A14|A10|A8|A4|A1|E1|G1|J1]

# incident 187
loadsgf games/incident187.sgf 227
3 gg_genmove white
#? [B15|A15|A17|B14]
4 gg_genmove black
#? [B19|A17]

# incident 216
loadsgf games/incident211.sgf 64
5 gg_genmove black
#? [PASS]
6 gg_genmove white
#? [PASS]

# incident 296
loadsgf games/incident291.sgf 274
7 gg_genmove black
#? [PASS]
8 gg_genmove white
#? [PASS]

loadsgf games/filllib1.sgf 54
9 gg_genmove white
#? [PASS]
10 gg_genmove black
#? [PASS]

loadsgf games/filllib1.sgf 58
11 gg_genmove white
#? [PASS]
12 gg_genmove black
#? [PASS]

loadsgf games/filllib1.sgf 60
13 gg_genmove white
#? [PASS]
14 gg_genmove black
#? [PASS]

loadsgf games/filllib2.sgf 312
15 gg_genmove white
#? [P19]
16 gg_genmove black
#? [R19]

loadsgf games/filllib3.sgf 105
17 gg_genmove white
#? [B11|A13]
18 gg_genmove black
#? [A10]

loadsgf games/filllib4.sgf 252
19 gg_genmove white
#? [A13]
20 gg_genmove black
#? [A13]

# White A7 may give a point if there are sufficient ko threats. The
# variation W A7, B A6, W D8, B A8, W B9, B A7 does not lose a point.
loadsgf games/filllib5.sgf 52
21 gg_genmove white
#? [A7|D9|B9]
22 gg_genmove black
#? [A7]

loadsgf games/filllib6.sgf 261
23 gg_genmove white
#? [P16]
24 gg_genmove black
#? [P16]

# White J3 is best but with superior ko threats, H1 and J5 can also be
# played with the same result.
loadsgf games/filllib7.sgf 72
25 gg_genmove white
#? [J3|H1|J5]
26 gg_genmove black
#? [J5|H7|J7]

loadsgf games/filllib8.sgf 303
27 gg_genmove white
#? [Q16]
28 gg_genmove black
#? [R14]

# It's not a very good idea to throw in to start a ko, but we accept
# it anyway.
loadsgf games/filllib9.sgf 235
29 gg_genmove white
#? [B1|A1]
30 gg_genmove black
#? [A3|A1|B1]

# Q19 for white is non-orthodox but doesn't lose points.
loadsgf games/trevor/auto/d29.sgf 188
31 gg_genmove white
#? [S16|T17|O18|Q19]
32 gg_genmove black
#? [O18|O19|N18|P19|N19]

loadsgf games/filllib10.sgf
33 gg_genmove white
#? [PASS]
34 gg_genmove black
#? [PASS]

# J2 inefficient
loadsgf games/nngs/speciman-gnugo-3.1.32-200204211014.sgf 53
35 gg_genmove black
#? [A8|B8]*
36 gg_genmove white
#? [A5|A8|B8]

loadsgf games/filllib11.sgf
37 gg_genmove white
#? [H5]
38 gg_genmove black
#? [E4]

loadsgf games/nngs/gnugo-3.1.34-guest1-200204242025.sgf 86
39 gg_genmove white
#? [G12|F12|G13]
40 gg_genmove black
#? [K13]

loadsgf games/nngs/gnugo-3.2-merlin-200205071828.sgf 244
41 gg_genmove white
#? [M8]*

loadsgf games/nngs/gnugo-3.3.11-rcde05-200211090008.sgf 271
42 gg_genmove white
#? [N19]
