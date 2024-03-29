# Reset node counters
reset_reading_node_counter
reset_owl_node_counter
reset_connection_node_counter

# Life and death test problems.
loadsgf games/life_and_death/ld1.sgf
1 dragon_status B18
#? [critical (A15|A18) (A15|A18)]
2 dragon_status S18
#? [critical (T18|O18) T18]
3 dragon_status S2
#? [critical S1 (S1|S7)]
4 dragon_status B2
#? [critical (A2|F2) (A2|F2)]

loadsgf games/life_and_death/ld2.sgf
5 dragon_status B18
#? [critical (B19|E19) (B19|E19|F18|F19)]
6 dragon_status S18
#? [dead]
7 dragon_status S2
#? [dead]
8 dragon_status B2
#? [critical A2 (A2|A5|B6|A6)]

loadsgf games/life_and_death/ld3.sgf
9 dragon_status B18
#? [critical A14 (A17|A14|B13|A13)]
10 dragon_status S18
#? [critical T15 (T15|S14|T14)]
11 dragon_status S2
#? [critical (S6|T6|T7|T2) S6]
# Attacking on F1 only gives a ko, which is a failure.
12 dragon_status B2
#? [critical B1 (E1|E2|B1|F1)]

loadsgf games/life_and_death/ld4.sgf
13 dragon_status B18
#? [dead]
14 dragon_status S18
#? [critical P18 (P18|R19)]
15 dragon_status S2
#? [critical T5 (S5|T6|T5)]
16 dragon_status B2
#? [critical A2 (B5|A6|A5|A2)]

loadsgf games/life_and_death/ld5.sgf
17 dragon_status B18
#? [critical A14 (A14|B17|A17|A18)]
18 dragon_status S18
#? [critical T17 (S17|T17|T18)]
19 dragon_status S2
#? [critical (S3|T5) (S3|T5)]
20 dragon_status B2
#? [critical (B3|A2|A6) (A2|B3)]

loadsgf games/life_and_death/ld6.sgf
21 dragon_status B18
#? [critical (A14|A17|B19) A17]
22 dragon_status S18
#? [critical T18 (T18|R18|R19|Q19)]
23 dragon_status S2
#? [dead]
24 dragon_status B2
#? [critical F1 (F1|D2|D1|B1)]

loadsgf games/life_and_death/ld7.sgf
25 dragon_status B18
#? [dead]
26 dragon_status S18
#? [alive]
27 dragon_status S2
#? [critical (Q2|T2|N2) Q1]
28 dragon_status B2
#? [alive]

loadsgf games/life_and_death/ld8.sgf
29 dragon_status B16
#? [critical C18 (C18|B18|D18)]
30 dragon_status S18
#? [critical Q18 (Q18|S19|R19|S14)]
31 dragon_status S4
#? [alive]
32 dragon_status B2
#? [critical (C1|A5|A2) (A5|C1|A2)]

loadsgf games/life_and_death/ld9.sgf
33 dragon_status C16
#? [dead]
34 dragon_status R16
#? [critical (Q19|S18|T18|T16) (T18|S18|Q19)]
35 dragon_status R3
#? [critical (T5|S2) (S2|T2|T3)]
36 dragon_status C4
#? [alive]

loadsgf games/life_and_death/ld10.sgf
37 dragon_status C16
#? [critical (A17|C19) (A17|C19|A16|A18)]
38 dragon_status R16
#? [dead]
39 dragon_status R3
#? [dead]
40 dragon_status C4
#? [critical (A5|B2) (B2|A2)]

loadsgf games/life_and_death/ld11.sgf
41 dragon_status C16
#? [critical B19 (B19|A16|B18|C19)]
42 dragon_status R16
#? [dead]
43 dragon_status R3
#? [critical (R1|S2|T7) S2]
# B2 lives but allows the opponent to make seki (or ko).
44 dragon_status C4
#? [critical D1 (A2|D1|B2)]

loadsgf games/life_and_death/ld12.sgf
45 dragon_status C16
#? [alive]
46 dragon_status R16
#? [alive]
47 dragon_status R3
#? [dead]
48 dragon_status C4
#? [alive]

loadsgf games/life_and_death/ld13.sgf
49 dragon_status C16
#? [critical B18 (A18|B18)]
50 dragon_status R16
#? [critical (T18|T16) (T18|S18|Q19)]
51 dragon_status R3
#? [critical S1 (R1|S1|T5|T4|T3|S2|S3|T1)]
# B1 is a better defense than D1 because the latter only lives by double ko.
52 dragon_status C4
#? [critical (D1|A5) (B1|D1|B2)]

loadsgf games/life_and_death/ld14.sgf
53 dragon_status C16
#? [critical (C19|B19|B18|A18) (A18|B18)]
54 dragon_status R16
#? [critical (T16|O19|T17|T18|S18) T18]
55 dragon_status R3
#? [critical (S2|T5|S3|T3|T2|R1|S1) S2]
56 dragon_status C4
#? [critical (D1|A5|A2) B1]

loadsgf games/life_and_death/ld15.sgf
57 dragon_status C16
#? [critical (A16|A17|A18|B18) A18]
58 dragon_status R16
#? [critical Q19 (T18|S18|Q19|S19)]
59 dragon_status R3
#? [alive]
60 dragon_status C4
#? [alive]

loadsgf games/life_and_death/ld16.sgf
61 dragon_status C16
#? [critical (B12|D13|D14|A14|A12|B11|C19|B18|A18|B19) (A18|B18|A14|B12)]
62 dragon_status R16
#? [alive]
63 dragon_status R3
#? [critical (S2|Q2|R2|R1|S1|T5) S2]
64 dragon_status C4
#? [alive]

loadsgf games/life_and_death/ld17.sgf
65 dragon_status B2
#? [critical A2 A2]

loadsgf games/life_and_death/ld18.sgf 1
66 dragon_status C9
#? [alive]

loadsgf games/life_and_death/ld18.sgf
67 dragon_status C9
#? [critical A9 A7]

loadsgf games/life_and_death/lunches.sgf
101 dragon_status A17
#? [dead]
102 dragon_status A11
#? [critical (C14|E14|F13|E11) E14]
103 dragon_status A5
#? [critical (A1|E7|M2|M1|N2|N1|O2|O1|P1) (A1|E7|M2|A8|C8|F4)]
104 dragon_status S1
#? [critical T6 (T6|T8)]
105 dragon_status H18
#? [dead]
106 dragon_status M7
#? [critical (O8|L8) O8]

# Life and death problems derived from the tripod group.
loadsgf games/life_and_death/tripod1.sgf
151 dragon_status C17
#? [alive]
152 dragon_status R17
#? [critical (S17|Q18|T16) T17]
153 dragon_status R3
#? [alive]
154 dragon_status C3
#? [critical (B3|D2|B1|A2) B1]

loadsgf games/life_and_death/tripod2.sgf
155 dragon_status C17
#? [critical (B16|B18) B18]
156 dragon_status R17
#? [alive]
157 owl_attack R3
#? [1 Q1]
158 owl_defend R3
#? [1 T1]
159 dragon_status C3
#? [critical (D2|A4) B1]

loadsgf games/life_and_death/tripod3.sgf
160 dragon_status C17
#? [critical B19 (B18|B19|C19)]
161 dragon_status R17
#? [dead]
162 dragon_status R3
#? [critical P1 R1]
163 owl_attack C3
#? [3 B1]
164 owl_defend C3
#? [1 (B2|B1|A2)]

loadsgf games/life_and_death/tripod4.sgf
165 owl_attack C17
#? [1 (A16|B18)]
166 owl_defend C17
#? [2 B18]
167 owl_attack R17
#? [3 T15]
168 owl_defend R17
#? [1 (R19|T16|T18|T19)]
169 owl_attack R3
#? [1 Q1]
170 owl_defend R3
#? [2 Q1]
171 owl_attack C3
#? [3 A2]
172 owl_defend C3
#? [1 (C1|A2)]

loadsgf games/life_and_death/tripod5.sgf
173 dragon_status C17
#? [critical (C18|D19|A16) C19]
174 dragon_status R17
#? [alive]
175 dragon_status R3
#? [critical (T4|S2) S2]
176 dragon_status C3
#? [alive]

loadsgf games/life_and_death/tripod6.sgf
# This is an extremely good ko for white. Black has to ignore 3 ko
# threats to be able to live.
177 owl_attack C17
#? [2 D19]
178 owl_defend C17
#? [1 (A19|A17)]
179 owl_attack R17
#? [3 T17]
180 owl_defend R17
#? [1 (Q18|T17|S18|T18|S17|S19)]
181 owl_attack R3
#? [1 (R2|S4|P1)]
182 owl_defend R3
#? [2 S1]
183 owl_attack C3
#? [3 B2]
184 owl_defend C3
#? [1 (B1|B2|B3|A4|D2)]

loadsgf games/life_and_death/tripod7.sgf
185 owl_attack C17
#? [1 (A19|A18)]
186 owl_defend C17
#? [2 (A18|C18)]
187 owl_attack R17
#? [1 (Q19|S19)]
188 owl_defend R17
#? [2 Q19]
189 dragon_status R3
#? [critical S1 (S4|S1|T2)]
190 dragon_status C3
#? [critical (B3|D2|B5|A5|A4) A2]

loadsgf games/life_and_death/tripod8.sgf
191 dragon_status C17
#? [alive]
192 dragon_status R17
#? [critical (T19|S19) (S17|S19)]
193 dragon_status R3
#? [critical (S4|T4|T5) S4]
194 owl_attack C3
#? [3 B1]
195 owl_defend C3
#? [1 (C1|A5|A3|A2|B1)]

loadsgf games/life_and_death/tripod9.sgf
196 owl_attack C17
#? [1 B19]
197 owl_defend C17
#? [2 B19]
198 dragon_status R17
#? [dead]
199 dragon_status R3
#? [alive]
200 dragon_status C3
#? [dead]

# More life and death problems around tripod group.

loadsgf games/life_and_death/tripod4.sgf
# Black tries a bad defense. He must be punished.
play black Q19
201 owl_attack Q19
#? [3 T15]

# Mixed life and death problems.
loadsgf games/life_and_death/mixed1.sgf
300 owl_attack  C17
#? [1 (E19|D19|C19|B18|B19|A18|A19)]
301 owl_defend  C17
#? [3 B18]
302 dragon_status R17
#? [critical T19 (Q19|R18)]
303 dragon_status R3
#? [critical (S3|Q3|Q1|T2|T3) S3]
304 dragon_status C3
#? [critical (A2|E1) A2]

loadsgf games/life_and_death/comb1.sgf
305 dragon_status C3
#? [critical E2 (E2|E1)]

loadsgf games/life_and_death/ld19.sgf
306 owl_attack C17
#? [1 B19]
307 owl_defend C17
#? [1 (E19|C19|B19|A18|A16)]
308 owl_attack R17
#? [1 T17]
309 owl_defend R17
#? [1 (T17|S19|Q19)]
310 owl_attack R3
#? [1 T2]
311 owl_defend R3
#? [1 (T2|R1|P1)]
312 owl_attack C3
#? [1 B1]
313 owl_defend C3
#? [1 (B1|A4)]

# Report number of nodes visited by the tactical reading
10000 get_reading_node_counter
#? [0]&

# Report number of nodes visited by the owl code
10001 get_owl_node_counter
#? [0]&

# Report number of nodes visited by the life code
10002 get_connection_node_counter
#? [0]&
