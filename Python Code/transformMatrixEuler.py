import math

m00 = 0.016376324546214
m01 = 0.023993958240586
m02 = 0.999578091446973
m10 = 0.386944343267803
m11 = -0.921968288811375
m12 = 0.015791042406242
m20 = 0.921957307246059
m21 = 0.386522650005368
m22 = -0.024383193555208

m00 = -0.037098
m01 = -0.015267
m02 = 0.999195
m10 = 0.388818
m11 = -0.921314
m12 = 0.000359 
m20 = 0.920567
m21 = 0.388517
m22 = 0.040115

if (m21 > 0.998):
	heading = 0
	attitude = math.atan2(m20,m22)
	bank = math.pi/2
elif (m21 < -0.998):
	heading = 0
	attitude = math.atan2(m20,m22)
	bank = math.pi/2
else:
	heading = math.atan2(-m12,m11);
	bank = math.atan2(-m20,m00);
	attitude = math.asin(m10);

conversion = 180/math.pi

headingY1 = -math.asin(m20)
headingY2 = math.pi - headingY1
attitudeX1 = math.atan2((m21/math.cos(headingY1)),(m22/math.cos(headingY1)))
attitudeX2 = math.atan2((m21/math.cos(headingY2)),(m22/math.cos(headingY2)))
bankZ1 = math.atan2((m10/math.cos(headingY1)),(m00/math.cos(headingY1)))
bankZ2 = math.atan2((m10/math.cos(headingY2)),(m00/math.cos(headingY2)))

W = math.atan2(m12,m22)
c2 = math.sqrt(m00*m00 + m01*m01)
P = math.atan2(-m02,c2)
s1 = math.sin(W)
c1 = math.cos(W)
R = math.atan2((s1*m20 - c1*m10), (c1*m11 - s1*m21))

print "\nprevious"
print heading * conversion
print attitude * conversion
print bank * conversion

print "\ntype 1"

print attitudeX1 * conversion
print headingY1 * conversion
print bankZ1 * conversion

print "\ntype 2"

print attitudeX2 * conversion
print headingY2 * conversion
print bankZ2 * conversion

print"\ntype 3"

print W * conversion
print P * conversion
print R * conversion