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

m00 = -0.070831976870960
m01 = 0.204724917157531
m02 = 0.976253511767251
m10 = 0.192882511470353
m11 = -0.957428623020512
m12 = 0.214771611380260
m20 = 0.978662045393321
m21 = 0.203515007854179
m22 = 0.028327935350747

if (m10 > 0.998):
	heading = math.atan2(m02,m22);
	attitude = math.pi/2;
	bank = 0;
elif (m10 < -0.998):
	heading = math.atan2(m02,m22);
	attitude = -math.pi/2;
	bank = 0;
else:
	heading = math.atan2(-m20,m00);
	bank = math.atan2(-m12,m11);
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