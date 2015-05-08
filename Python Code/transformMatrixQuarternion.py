import math

# code to test matrix to quarternion conversion

m00 = -0.735425
m01 = 0.662211
m02 = 0.143618
m10 = -0.568151
m11 = -0.718123
m12 = 0.401876
m20 = 0.369262
m21 = 0.213953
m22 = 0.904362

tr = m00 + m11 + m22

if (tr > 0): 
	S = math.sqrt(tr+1.0) * 2
	qw = 0.25 * S
	qx = (m21 - m12) / S
  	qy = (m02 - m20) / S
  	qz = (m10 - m01) / S
elif ((m00 > m11) and (m00 > m22)):
	S = math.sqrt(1.0 + m00 - m11 - m22) * 2
	qw = (m21 - m12) / S
	qx = 0.25 * S
	qy = (m01 + m10) / S
	qz = (m02 + m20) / S
elif (m11 > m22):
	S = math.sqrt(1.0 + m11 - m00 - m22) * 2
	qw = (m02 - m20) / S
	qx = (m01 + m10) / S
	qy = 0.25 * S
	qz = (m12 + m21) / S
else: 
	S = math.sqrt(1.0 + m22 - m00 - m11) * 2
	qw = (m10 - m01) / S
	qx = (m02 + m20) / S
	qy = (m12 + m21) / S
	qz = 0.25 * S;

print "qw: " + str(qw)
print "qx: " + str(qx)
print "qy: " + str(qy)
print "qz: " + str(qz)