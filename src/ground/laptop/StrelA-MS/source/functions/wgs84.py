import math

DR2D = 57.29577951308232087679815

a = 6378137.0
f = 1.0 / 298.257223563

aeps2 = a*a* 1e-32 
e2 = (2.0 - f) * f
e4t = e2*e2 * 1.5
ec2 = 1.0 - e2
ec = math.sqrt(ec2)
b = a * ec

def wgs84_xyz_to_latlonh(x, y, z):
   p2 = x*x + y*y

   if (p2 > 0):
      lon = math.atan2(y, x) * DR2D
   else:
      lon = 0

   absz = math.fabs(z)

   if (p2 > aeps2):
      p = math.sqrt(p2)

      s0 = absz / a
      pn = p / a
      zc = ec * s0

      c0 = ec * pn
      c02 = c0 * c0
      c03 = c02 * c0
      s02 = s0 * s0
      s03 = s02 * s0
      a02 = c02 + s02
      a0 = math.sqrt(a02)
      a03 = a02 * a0
      d0 = zc*a03 + e2*s03
      f0 = pn*a03 - e2*c03

      b0 = e4t * s02 * c02 * pn * (a0 - ec)
      s1 = d0*f0 - b0*s0
      cc = ec * (f0*f0 - b0*c0)

      lat = math.atan(s1/cc) * DR2D
      s12 = s1 * s1
      cc2 = cc * cc
      height = (p*cc + absz*s1 - a * math.sqrt(ec2*s12 + cc2)) / math.sqrt(s12 + cc2)
   else:
      lat = 90
      height = absz - b

   if (z < 0):
      lat = -lat

   return [lat, lon, height]


if __name__ == "__main__":
   print(wgs84_xyz_to_latlonh(2830356.5, 2197847.9, 5258838.4))