import math

qw = 0.101310386222
qx = 0.0436887824215
qy = -0.0400768118879
qz = 0.993086728337

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