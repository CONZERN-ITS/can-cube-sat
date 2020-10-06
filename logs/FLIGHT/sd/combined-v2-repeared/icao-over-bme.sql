drop table if exists icao_over_bme;

create table icao_over_bme(
    bme_fid integer,
    icao_fid integer,
    icao_alt real
);

insert into icao_over_bme (bme_fid, icao_fid, icao_alt)
select bme.OGC_FID, icao.OGC_FID, icao.alt
from 'pld_bme280_data_13_0' as bme
join 'icao_table' icao on icao.OGC_FID = ( 
    select icao2.OGC_FID from 'icao_table' as icao2
    join 'pld_bme280_data_13_0' as bme2
    where bme2.OGC_FID = bme.OGC_FID
    order by abs(bme2.pressure - icao2.pressure) asc
    limit 1
)
;

update 'pld_bme280_data_13_0'
set (icao_alt)
 = (
     select icao_alt
     from icao_over_bme
     where icao_over_bme.bme_fid = "pld_bme280_data_13_0".OGC_FID
);
