# Reset reading node counter
reset_reading_node_counter

# incident 2
loadsgf games/incident2.sgf 212
1 attack E3
#? [0]

# These are all trivialities.
2 attack F1
#? [1 G1]
3 defend F1
#? [1 G1]
4 attack E19
#? [2 D19]
5 defend E19
#? [1 D19]
6 attack S5
#? [1 (S4|T5)]
7 defend S5
#? [0]
8 attack O3
#? [1 O4]
9 defend O3
#? [0]

# incident 79
loadsgf games/incident79.sgf 168
10 attack L19
#? [0]

11 attack K18
#? [1 J17]
# FIXME: Not sure whether defense at J16 works without ko.
12 defend K18
#? [1 J16]

# incident 67 (slightly modified)
loadsgf games/incident67.sgf 21
13 defend G3
#? [1 H1]

# incident 64
loadsgf games/incident64.sgf 52
14 attack F7
#? [0]
15 attack F9
#? [1 G9]
16 defend F9
#? [0]

# incident 55
loadsgf games/incident55.sgf 98
17 defend L5
#? [0]

# incident 80
loadsgf games/incident80.sgf 141
18 attack T10
#? [0]

# incident 82
loadsgf games/incident82.sgf 184
19 attack B16
#? [1 A14]

# incident 114
loadsgf games/incident114.sgf 36
20 attack S4
#? [1 (R4|T4|T5)]

# incident 231
# Three space nakade in the corner.
loadsgf games/incident231.sgf 206
21 attack B4
#? [1 A1]

# incident 84 (slightly modified)
loadsgf games/incident84.sgf 57
22 defend A8
#? [1 (B5|A7)]

# incident 86
loadsgf games/incident85.sgf 214
23 attack H17
#? [0]

# incident 92
# Double snapback. Also see tests 89, 90.
loadsgf games/incident92.sgf 250
24 attack O2
#? [1 Q1]

# incident 93
# Defense available by backfilling. Also see test 91.
loadsgf games/incident93.sgf 42
25 attack E1
#? [0]

# incident 144
loadsgf games/incident144.sgf 98
increase_depths
trymove W F17
26 defend J19
#? [1 G19]
popgo
decrease_depths

# incident 42
# Indirect defense causing shortage of liberties available. See also
# test 92.
loadsgf games/incident42.sgf 58
increase_depths
trymove B S15
27 defend P16
#? [1 T15]
popgo
decrease_depths

# incident 43
# Essentially identical to incident 42. See also test 92.
loadsgf games/incident42.sgf 88
increase_depths
trymove B S15
28 defend P16
#? [1 T15]
popgo
decrease_depths

# incident 118
# This is a ko reading mistake. See also tests 93--96.
loadsgf games/incident118.sgf 252
29 attack J10
#? [1 (F1|F2|N1)]
30 defend J10
#? [2 J1]

# incident 119
# This is a ko reading mistake. See also tests 97 and 98.
loadsgf games/incident118.sgf 256
# Technically N1 also works with ko, but it's inferior since we have
# to win the ko twice.
31 attack J10
#? [3 (F1|F2)]
# This test passes if ko depth is increased to 9.
32 defend J10
#? [2 J1]

# incident 125
loadsgf games/incident121.sgf 74
33 attack O3
#? [1 P3]
34 defend O3
#? [1 (P3|O2|P2)]

# incident 239
# Ko is involved here too, if trying to defend at B1.
loadsgf games/incident239.sgf 58
35 attack A3
#? [1 B1]
36 defend A3
#? [1 A5]

# incident 167b
# Another ko reading mistake.
loadsgf games/incident165.sgf 212
37 attack J14
#? [2 (L11|K9|L9|N4)]
38 defend J14
#? [1 (N4|O2|P2|N1)]*

# This is a reading mistake from the first game of the Honinbo title
# match 2000, between O Meien (white) and Cho Sonjin (black). White
# resigned after move 59.
loadsgf games/ladder1.sgf 59
39 attack G12
#? [1 K13]

# The same position a few moves earlier.
loadsgf games/ladder1.sgf 55
40 attack G12
#? [1 H12]

# incident 185
# This mistake can be solved by increasing the backfill depth to 11.
loadsgf games/incident185.sgf 204
41 defend O19
#? [0]

# incident 186
loadsgf games/incident186.sgf
42 attack E9
#? [1 F9]

# incident 194 (slightly modified)
loadsgf games/incident189.sgf 222
43 attack S12
#? [0]

# incident 195
loadsgf games/incident189.sgf 252
44 attack A9
#? [0]

# incident 225
# Ko reading mistake.
loadsgf games/incident225.sgf 252
45 defend H18
#? [2 H17]

# incident 226
# Ko reading mistake.
loadsgf games/incident225.sgf 254
46 defend H18
#? [2 H17]

# incident 246
loadsgf games/incident246.sgf 74
increase_depths
trymove B L9
47 defend M12
#? [1 M11]
48 defend M10
#? [1 M11]
49 attack N11
#? [1 M11]
popgo
decrease_depths

# incident 247
loadsgf games/incident246.sgf 76
50 attack M12
#? [0]

# This is a cleaned up version of incident 247
loadsgf games/reading01.sgf
51 attack D6
#? [0]

# This is a simplified version of the reading problem in incident 59.
# Still it's a very tough problem for a computer.
loadsgf games/reading02.sgf
52 attack C3
#? [1 H2]*
53 attack D1
#? [1 H2]*
54 defend E2
#? [1 H2]

loadsgf games/owl03.sgf
55 attack D19
#? [1 E19]*

loadsgf games/reading03.sgf
# Should find ko attack. Also a life and death problem
56 attack J7
#? [3 J8]

# incident 252
loadsgf games/incident252.sgf 134
57 attack H3
#? [0]

# incident 272
# This mistake can be solved by increasing the fourlib depth to 11.
loadsgf games/incident272.sgf 24
58 defend M17
#? [0]

# incident 276
# This ko is quite messy with respect to internal ko threats. Awaiting
# further analysis, we accept both return codes 2 and 3.
loadsgf games/incident272.sgf 252
59 defend N2
#? [(2|3) P5]

# incident 280
loadsgf games/incident278.sgf 160
60 defend F16
#? [0]

loadsgf games/strategy5.sgf 44
61 defend S5
#? [1 S3|S4]

loadsgf games/reading04.sgf 237
trymove B E1
62 defend D6
#? [2 (F2|G3)]
popgo

# Position from Tim's game
loadsgf games/reading05.sgf
63 defend A9
#? [0]

loadsgf games/nicklas/nicklas11.sgf 244
64 defend P8
#? [3 N1]

loadsgf games/reading06.sgf
65 defend S5
#? [0]*

# Ko tests.
loadsgf games/reading07.sgf 1
66 attack G9
#? [2 E7]
67 defend F8
#? [2 E7]
68 defend E9
#? [2 E7]

loadsgf games/reading07.sgf 2
69 defend G9
#? [3 E8]
70 attack F8
#? [3 E8]
71 attack E9
#? [3 E8]

# A simplification of incident 225. Ko tests.
loadsgf games/reading08.sgf
72 attack G9
#? [1 E6]
73 defend G9
#? [1 E6]
74 attack E8
#? [3 E9]
75 defend E8
#? [1 (E6|E9|D9|D8)]
76 attack C9
#? [1 E6]
77 defend C9
#? [1 E6]

# A simplification of incident 119. Ko tests.
loadsgf games/reading09.sgf 1
78 attack C5
#? [3 (C1|C2)]
79 defend C5
#? [2 F1]
80 attack E5
#? [2 F1]
81 defend E5
#? [3 (C1|C2)]

# More ko tests.
loadsgf games/reading09.sgf 3
82 attack C5
#? [3 G1]
83 defend C5
#? [1 F3]
84 attack E5
#? [1 F3]
85 defend E5
#? [3 G1]

# Failure to consider double atari on surrounding chain.
loadsgf games/reading10.sgf
86 defend E5
#? [0]

# Crawling along the edge.
loadsgf games/strategy5.sgf 45
87 attack T7
#? [1 S4]

# Necessary to start by reinforcing surrounding chain.
loadsgf games/reading11.sgf
88 attack G6
#? [1 (D5|E4)]

# Simplification of test 24, double snapback.
loadsgf games/reading12.sgf
89 attack D2
#? [1 F1]
90 attack H2
#? [1 F1]

# Simplification of test 25, backfilling defense.
loadsgf games/reading12.sgf
91 attack A6
#? [0]

# Simplification of tests 27 and 28, indirect defense.
loadsgf games/incident42.sgf 89
92 defend P16
#? [1 T15]

# Simplification of tests 29 and 30, ko mistakes.
loadsgf games/reading13.sgf 1
93 attack B5
#? [1 (A3|E6)]
94 defend B5
#? [2 D1]

# Ko mistake
loadsgf games/reading13.sgf 2
95 attack B5
#? [3 E1]

# Ko mistake
loadsgf games/reading13.sgf 3
96 defend B5
#? [2 F1]

# Simplification of tests 31 and 32, ko mistakes.
loadsgf games/reading13.sgf 4
97 attack B5
#? [3 (A3|E6)]
98 defend B5
#? [2 D1]

# Well-known tesuji.
loadsgf games/reading14.sgf
99 attack D4
#? [1 D6]

# Tests for net3
loadsgf games/net3.sgf
100 attack J18
#? [1 H16]
101 attack L18
#? [1 M16]
102 attack B11
#? [1 D12]
103 attack B9
#? [1 D8]
104 attack R11
#? [1 Q12]
105 attack R9
#? [1 Q8]
106 attack J3
#? [1 H4]
107 attack L3
#? [1 M4]

loadsgf games/reading16.sgf
# test for draw_back
108 attack L3
#? [1 K2]
109 defend L3
#? [1 (K2|M2|N3)]

# Simplification of test 21.
loadsgf games/reading12.sgf
110 attack G9
#? [1 J9]
111 defend G9
#? [1 J9]

# Well-known tesuji.
loadsgf games/reading15.sgf
112 attack H5
#? [1 H2]
113 defend H5
#? [1 H2]
114 attack A5
#? [1 (A3|B2)]
115 defend A5
#? [1 B2]
116 attack B9
#? [1 (D9|E8)]
117 defend B9
#? [1 E8]
118 attack G9
#? [1 (D9|E8)]
119 defend G9
#? [1 E8]

# Small scale semeai.
loadsgf games/reading17.sgf
120 attack E8
#? [1 C7]

# Ko can be avoided.
loadsgf games/reading18.sgf
121 attack E5
#? [1 C4]

# Small scale semeai.
loadsgf games/reading18.sgf
122 defend J9
#? [1 (F7|F9)]

# based on test 12 (incident79.sgf)
loadsgf games/reading19.sgf
123 attack K18
#? [1 G19]

# based on strategy test 18
loadsgf games/reading20.sgf
124 attack G3
#? [1 K3]

# Ko can not be avoided.
loadsgf games/reading21.sgf
125 attack D1
#? [2 E1]
126 defend D1
#? [1 E1]
127 attack D9
#? [1 F7]
128 defend D9
#? [3 F7]

# Backfilling insufficient, need to back-capture.
loadsgf games/reading22.sgf
129 attack H2
#? [1 (F5|F6)]

loadsgf games/TSa.sgf 298
130 attack E19
#? [0]

# Derived from strategy2 test 78. See also test case 138.
loadsgf games/strategy21.sgf 96
trymove black H2
131 attack F4
#? [0]
popgo

loadsgf games/reading22.sgf
132 attack J8
#? [2 J9]
133 defend J8
#? [3 F9]

# See also test case 137.
loadsgf games/strategy27.sgf 62
134 attack H2
#? [1 J2]*

loadsgf games/nicklas/nicklas18.sgf 45
135 attack A7
#? [2 A6]

loadsgf games/explorer.sgf 64
136 attack S8
#? [2 (T6|T12)]

loadsgf games/reading23.sgf
137 attack C2
#? [1 (H2|G1)]
138 defend E8
#? [2 F9]*

# incident 156
loadsgf games/incident156.sgf 186
139 attack B13
#? [0]

loadsgf games/reading24.sgf 48
140 attack E2
#? [0]

# superstring problem with double ko
loadsgf games/reading26.sgf
141 attack D1
#? [0]*

loadsgf games/reading27.sgf 66
142 attack N16
#? [1 O16]

loadsgf games/reading28.sgf
143 attack F9
#? [1 (D9|G8)]
144 defend F9
#? [0]

loadsgf games/ko5.sgf
145 attack J13
#? [1 H12]
146 defend J13
#? [2 H12]
147 attack M3
#? [1 H1]*
148 defend M3
#? [2 K2]

loadsgf games/gwe.sgf 225
149 attack T1
#? [0]

loadsgf games/reading29.sgf
150 attack B18
#? [1 B17]*

loadsgf games/reading29.sgf
151 attack M2
#? [1 M3]

loadsgf games/reading30.sgf
152 attack L18
#? [1 (K18|L17|M18)]

loadsgf games/reading31.sgf
153 attack C3
#? [0]

# Ko depth needs to be increased to 11 before the ko is detected.
loadsgf games/trevor/trevor_03.sgf 29
154 attack E4
#? [2 G4]*

loadsgf games/reading32.sgf
155 attack G9
#? [0]

loadsgf games/reading33.sgf
156 attack B3
#? [1 B2]*
157 attack R3
#? [0]

loadsgf games/strategy11.sgf 77
158 attack H2
#? [1 G2]*

loadsgf games/trevor/auto/d12.sgf 66
159 attack B14
#? [0]

loadsgf games/ssstator.sgf 133
trymove black C1
160 attack D1
#? [0]
popgo

loadsgf games/reading34.sgf
161 attack C9
#? [0]

loadsgf games/reading34.sgf
162 attack D7
#? [0]

# See also trevorb:840.
loadsgf games/trevor/auto/b75.sgf 62
163 attack E12
#? [0]

loadsgf games/reading35.sgf
164 defend B6
#? [1 C7]*

# See also 13x13:78
loadsgf games/reading36.sgf 23
165 attack L4
#? [1 K3|L3]*

# See also semeai:36
loadsgf games/nngs/Lazarus-gnugo-3.1.19-200201092246.sgf 66
166 attack R11
#? [1 R12]*

# Gets it right if F5 is filled, so the issue is finding the
# backfilling move.
loadsgf games/doubleko.sgf
167 attack J1
#? [1 F5]*

# This position can reveal komaster scheme weaknesses.
loadsgf games/reading37.sgf
168 defend B5
#? [2 B2]

# See also trevorb:430
# After B:F13, W:D13, B:H13, W:G13, B:G12, white G11 would be self-atari.
loadsgf games/trevor/auto/b28.sgf 80
169 defend F12
#? [1 F13]

# See also owl:237
loadsgf games/owl33.sgf 212
170 attack S13
#? [0]

# See also connect:4,54,55
loadsgf golois/Basique990715-1.sgf
trymove W Q4
trymove B R4
trymove W P4
trymove B R5
trymove W R3
171 attack R3
#? [1 R2]
popgo
popgo
popgo
popgo
popgo

loadsgf games/reading38.sgf
172 defend N14
#? [1 Q10]

# See also nngs2:1.
loadsgf games/nngs/gnugo-3.3.2-Zahlman-200205221717.sgf 207
trymove white A4
173 defend B2
#? [1 (A2|C7|B1)]*
popgo

# Although gnugo plays N16, the fact it doesn't see that the move
# is also a defense for N13 causes owl misreads.
loadsgf games/nngs/Lazarus-gnugo-3.1.19-200201092246.sgf 258
trymove black T8
trymove white P16
174 defend N13
#? [1 N16]*
popgo
popgo

loadsgf games/reading39.sgf
175 defend O3
#? [1 (N1|N2)]

# See also nngs3:1170
loadsgf games/nngs/gnugo-3.3.9-nailer-200210192227.sgf 242
trymove white B1
176 attack B1
#? [0]*
popgo

loadsgf games/mertin13x13/katsunari-gnugo2.W+4.sgf 126
trymove black L4
trymove white N3
trymove black N2
177 defend K4
#? [1 N1]*
popgo
popgo
popgo


# Report number of nodes visited by the tactical reading
10000 get_reading_node_counter
#? [0]&
