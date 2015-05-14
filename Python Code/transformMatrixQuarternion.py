import math

# code to test matrix to quarternion conversion

m00 = -0.975655
m01 = -0.204722
m02 = 0.078653
m10 = 0.197718
m11 = -0.976260
m12 = -0.088452
m20 = 0.094894
m21 = -0.070747
m22 = 0.992970

m00 = 0.016022399585789
m01 = -0.679107625557264
m02 = 0.733864012146070
m10 = 0.536845932297598
m11 = -0.613351316866585
m12 = -0.579307686028109
m20 = 0.843527866626844
m21 = 0.403254150106357
m22 = 0.354749315684513

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