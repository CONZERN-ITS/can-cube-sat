import time
import gps

class GPS_data():
    def __init__(self, timeout=1):
        self.timeout = timeout

    def setup(self, mode=gps.WATCH_ENABLE|gps.WATCH_JSON):
        self.gpsd = gps.gps(**{"mode":mode})

    def find_tpv_data(self):
        start = time.time()
        while ((time.time() - start) < self.timeout):
            data = self.gpsd.next()
            if data['class'] == 'TPV':
                return data
        return None

    def tpv_get_lat_lon(self, data):
        lan_lot = (getattr(data,'lat', None),
                   getattr(data,'lon', None))

        for element in lan_lot:
            if element is None:
                return None 
        return lan_lot

    def tpv_get_alt(self, data):
        alt = (getattr(data,'alt', None))

        if alt is None:
            return None 
        return alt

    def tpv_get_x_y_z(self, data):
        x_y_z = (getattr(data,'ecefx', None),
                 getattr(data,'ecefy', None),
                 getattr(data,'ecefz', None))

        for element in x_y_z:
            if element is None:
                return None 
        return x_y_z