
loadsgf games/handtalk/handtalk1.sgf 113
3 owl_attack q19
#? [2 T15]*

loadsgf games/handtalk/handtalk2.sgf 35
4 owl_defend r8
#? [1 P10]*

loadsgf games/handtalk/handtalk2.sgf 70
5 owl_defend p18
#? [2 (S18|O17|N18)]*

# O18 might work, but I don't think so. /ab
loadsgf games/handtalk/handtalk2.sgf 78
6 owl_defend P19
#? [1 (N18|O17)]*

7 owl_analyze_semeai Q17 P19
#? [ALIVE DEAD (O18|N18|O17|P17)]

8 owl_analyze_semeai P19 Q17
#? [ALIVE DEAD (N18|O17|S15)]

loadsgf games/handtalk/handtalk2.sgf 134
9 owl_attack r8
#? [1 (T6|T7)]

loadsgf games/handtalk/handtalk2.sgf 136
10 owl_defend q4
#? [0]

#loadsgf games/handtalk/handtalk2.sgf 160
#11 owl_defend g15
##? [1 E13]*
