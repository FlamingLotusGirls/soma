# Adapted from: http://jared.geek.nz/2013/feb/linear-led-pwm

INPUT_SIZE = 255       # Input integer size
OUTPUT_SIZE = 255      # Output integer size
INT_TYPE = 'uint8_t'
TABLE_NAME = 'cie';

def cie1931(L):
    L = L*100.0
    if L <= 8:
        return (L/902.3)
    else:
        return ((L+16.0)/116.0)**3

x = range(0,int(INPUT_SIZE+1))
brightness = [cie1931(float(L)/INPUT_SIZE) for L in x]

numerator = 1
denominator = 255
on = []
off = []

for bright in brightness:
	while float(numerator) / float(denominator) < bright:
		if numerator >= 128:
			numerator += 1
		else: 
			denominator -= 1

		if denominator == 128:
			denominator = 255
			numerator *= 2
	on.append(numerator)
	off.append(denominator)

# for i in range(256):
# 	print on[i], " / ", off[i]



f = open('gamma_correction_table.h', 'w')
f.write('// CIE1931 correction table\n')
f.write('// Automatically generated\n\n')

f.write('%s %s[%d] = {\n' % (INT_TYPE, "on", INPUT_SIZE+1))
f.write('\t')
for i,L in enumerate(on):
    f.write('%d, ' % int(L))
    if i % 10 == 9:
        f.write('\n\t')
f.write('\n};\n\n')
f.write('%s %s[%d] = {\n' % (INT_TYPE, "off", INPUT_SIZE+1))
f.write('\t')
for i,L in enumerate(off):
    f.write('%d, ' % int(L))
    if i % 10 == 9:
        f.write('\n\t')
f.write('\n};\n\n')