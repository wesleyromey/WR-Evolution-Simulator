s = sun = energy is from the sun
g = gnd = energy is from the ground
c = cells (or carnivore) = energy is from eating other cells ("carnivore")
balanced = energy comes from the sun, ground, and other cells

sunScore = EAM[EAM_SUN] / 15 (rounded down)
gndScore = EAM[EAM_SUN] / 15 (rounded down)
carnScore = EAM[EAM_SUN] / 15 (rounded down)

Represent the scores as {sunScore}{gndScore}{carnScore}
	Anything above 5 is regarded as 5
Represent the file as {topLeft}{topRight}{bottomLeft}{bottomRight}
	0 = balanced, 1 = sun, 2 = gnd, 3 = carn

1. Set the score of each EAM to 0 if EAM[...] < 15

2. Figure out the obvious cases
	If max(EAM) <= 2*min(EAM), then balanced
		Guaranteed that min(EAM) >= 25 || max(EAM) <= 40
	If max(EAM) >= 75 || max(EAM) <, then all 1 color
		
	If min(EAM) >= 20, then balanced with the most-occurring color
	If min(EAM) <= 2*

49, 24, 24

41, 24, 24

41,30,14

1. Convert each score to whole number ratios (scores < 10 convert to 0)
	with the each score divided by the minimum score (rounded down)

Suppose that EAM[EAM_SUN] >= EAM[EAM_GND] >= EAM[EAM_CELLS]

X00: 


0: (0-14), (0-28)
1: (15-29), (30-58)
2: (30-44), (60-88)
3: (45-59), (90-100)
4: (60-74)
5: (75-89)
6: (90-100)

00X | 3333
0X0 | 2222
X00 | 1111
XX5, XX6 | 3333
X5X, X6X | 2222
5XX, 6XX | 1111
004, 014, 104, 114 | 3333
040, 041, 140, 141 | 2222
400, 401, 410, 411 | 1111

011 | 
012 | 
013 | 
014 | 





