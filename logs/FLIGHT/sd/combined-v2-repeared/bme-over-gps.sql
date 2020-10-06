drop table if exists bme_over_gps;

create table bme_over_gps(
    gps_fid integer,
    bme_fid integer,
    bme_temperature real,
    bme_pressure real,
    bme_humidity real
);

insert into bme_over_gps (gps_fid, bme_fid, bme_temperature, bme_pressure, bme_humidity)
select
    gps.OGC_FID,
    bme.OGC_FID,
    bme.temperature,
    bme.pressure,
    bme.humidity
from 'GPS_UBX_NAV_SOL-11-0' as gps
join 'pld_bme280_data_13_0' bme on bme.OGC_FID = ( 
    select bme2.OGC_FID from 'pld_bme280_data_13_0' as bme2
    join 'GPS_UBX_NAV_SOL-11-0' gps2
    where gps2.OGC_FID = gps.OGC_FID
    order by abs(gps2.minutes - bme2.time_from_start_min) asc
    limit 1
)
;

update "GPS_UBX_NAV_SOL-11-0"
set (bme_temperature, bme_pressure, bme_humidity)
 = (
     select bme_temperature, bme_pressure, bme_humidity
     from bme_over_gps
     where bme_over_gps.gps_fid = "GPS_UBX_NAV_SOL-11-0".OGC_FID
);