drop table if exists mics_over_gps;

create table mics_over_gps(
    gps_fid integer,
    mics_fid integer,
    mics_co real,
    mics_no2 real,
    mics_nh3 real
);

insert into mics_over_gps (gps_fid, mics_fid, mics_co, mics_no2, mics_nh3)
select
    gps.OGC_FID,
    mics.OGC_FID,
    mics.co_conc,
    mics.no2_conc,
    mics.nh3_conc
from 'GPS_UBX_NAV_SOL-11-0' as gps
join 'pld_mics6814_data' as mics on mics.OGC_FID = ( 
    select mics2.OGC_FID from 'pld_mics6814_data' as mics2
    join 'GPS_UBX_NAV_SOL-11-0' gps2
    where gps2.OGC_FID = gps.OGC_FID
    order by abs(gps2.minutes - mics2.time_from_start_min) asc
    limit 1
)
;

update "GPS_UBX_NAV_SOL-11-0"
set (mics_co, mics_no2, mics_nh3)
 = (
     select mics_co, mics_no2, mics_nh3
     from mics_over_gps
     where mics_over_gps.gps_fid = "GPS_UBX_NAV_SOL-11-0".OGC_FID
);