import math

qw = -0.335713176869
qx = 0.139943020146
qy = 0.168033399925
qz = 0.916230047532

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
print heading
print attitude
print str(bank) + "\n"

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
print heading
print attitude
print bank