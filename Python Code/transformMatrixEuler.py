import math

m00 = -0.735425
m01 = 0.662211
m02 = 0.143618
m10 = -0.568151
m11 = -0.718123
m12 = 0.401876
m20 = 0.369262
m21 = 0.213953
m22 = 0.904362

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

print heading
print attitude
print bank