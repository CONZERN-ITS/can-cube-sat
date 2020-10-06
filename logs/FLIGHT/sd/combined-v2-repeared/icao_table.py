from ambiance import Atmosphere

print("alt,pressure,temperature")
for level in range(-300, 30000, 1):
    atm = Atmosphere(level)
    print("%s,%s,%s" % (level, atm.pressure[0], atm.temperature[0]))
