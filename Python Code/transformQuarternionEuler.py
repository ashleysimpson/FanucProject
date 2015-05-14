import math

qw = 0.109893169679
qx = 0.677588911418
qy = 0.1233869506
qz = 0.716639678586

qw = 0.435149393694
qx = 0.564496164538
qy = -0.0630037133566
qz = 0.698583966493

conversion = 180 / math.pi

test = qx*qy + qz*qw;

if (test > 0.499):
	heading = 2 * math.atan2(qx,qw);
	attitude = math.pi/2;
	bank = 0;
elif (test < -0.499):
	heading = -2 * math.atan2(qx,qw);
	attitude = - math.pi/2;
	bank = 0;
else:
	sqx = qx*qx
	sqy = qy*qy
	sqz = qz*qz

	heading = math.atan2(2*qy*qw-2*qx*qz , 1 - 2*sqy - 2*sqz)
	attitude = math.asin(2*test)
	bank = math.atan2(2*qx*qw-2*qy*qz , 1 - 2*sqx - 2*sqz)

print "Normalised: "
print heading * conversion
print attitude * conversion
print str(bank * conversion) + "\n"

sqx = qx*qx
sqy = qy*qy
sqz = qz*qz
sqw = qw*qw

unit = sqx + sqy + sqz + sqw

if (test > 0.499*unit):
	heading = 2 * math.atan2(qx,qw)
	attitude = math.pi/2
	bank = 0
elif (test < -0.499*unit):
	heading = -2 * math.atan2(qx,qw)
	attitude = -math.pi/2
	bank = 0
else:
	heading = math.atan2(2*qy*qw-2*qx*qz , sqx - sqy - sqz + sqw)
	attitude = math.asin(2*test/unit)
	bank = math.atan2(2*qx*qw-2*qy*qz , -sqx + sqy - sqz + sqw)

print "Non-Normalised: "
print heading * conversion
print attitude * conversion
print bank * conversion

# go through all 6 possible versions for quarternions
allSixConversions = 0
W = 0
P = 0
R = 0

while (allSixConversions < 6):
	print ""

	# XYZ
	if (allSixConversions == 0):
		print "XYZ"
		e = -1
		p0 = qw
		p1 = qx
		p2 = qy
		p3 = qz
	# YZX
	elif (allSixConversions == 1):
		print "YZX"
		e = -1
		p0 = qw
		p1 = qy
		p2 = qz
		p3 = qx
	# ZXY
	elif (allSixConversions == 2):
		print "ZXY"
		e = -1
		p0 = qw
		p1 = qz
		p2 = qx
		p3 = qy
	# ZYX
	elif (allSixConversions == 3):
		print "ZYX"
		e = 1
		p0 = qw
		p1 = qz
		p2 = qy
		p3 = qx
	# XZY
	elif (allSixConversions == 4):
		print "XZY"
		e = 1
		p0 = qw
		p1 = qx
		p2 = qz
		p3 = qy
	# YXZ
	elif (allSixConversions == 5):
		print "YXZ"
		e = 1
		p0 = qw
		p1 = qy
		p2 = qx
		p3 = qz
	# XZX
	elif (allSixConversions == 6):
		print "XZX"
		e = 1
		p0 = qw
		p1 = qx
		p2 = qz
		p3 = qx
	# XYX
	elif (allSixConversions == 7):
		print "XYX"
		e = 1
		p0 = qw
		p1 = qx
		p2 = qy
		p3 = qx
	# YXY
	elif (allSixConversions == 8):
		print "YXY"
		e = 1
		p0 = qw
		p1 = qy
		p2 = qx
		p3 = qy
	# YZY
	elif (allSixConversions == 9):
		print "YZY"
		e = 1
		p0 = qw
		p1 = qy
		p2 = qz
		p3 = qy
	# ZYZ
	elif (allSixConversions == 10):
		print "ZYZ"
		e = 1
		p0 = qw
		p1 = qz
		p2 = qy
		p3 = qz
	# ZXZ
	else:
		print "ZXZ"
		e = 1
		p0 = qw
		p1 = qz
		p2 = qx
		p3 = qz

	test = (p0*p2) + (e*p1*p3);

	if (math.asin(2*test) > 0.499):
		W = math.atan2(p1,p0);
		P = (math.pi/2);
		R = 0;
	elif (math.asin(2*test) < -0.499):
		W = math.atan2(p1,p0);
		P = - (math.pi/2);
		R = 0;
	else:
		sp1 = p1*p1
		sp2 = p2*p2
		sp3 = p3*p3

		W = math.atan2(2*((p0*p1) - (e*p2*p3)), 1 - (2*(sp1 + sp2)))
		P = math.asin(2*test)
		R = math.atan2(2*((p0*p3) - (e*p1*p2)), 1 - (2*(sp2 + sp3)))

	print W * conversion
	print P * conversion
	print R * conversion

	allSixConversions += 1
