insert into tmp1 (gps_fid, o2_fid, o2_conc)
select gps.OGC_FID, o2.OGC_FID, o2.o2_conc from 'GPS_UBX_NAV_SOL-11-0' as gps
join 'PLD_ME2O2_DATA-13-0' o2 on o2.OGC_FID = ( 
    select o22.OGC_FID from 'PLD_ME2O2_DATA-13-0' as o22
    join 'GPS_UBX_NAV_SOL-11-0' gps2
    where gps2.OGC_FID = gps.OGC_FID
    order by abs(o22.time_from_start_min - gps2.minutes) asc
    limit 1
)
;