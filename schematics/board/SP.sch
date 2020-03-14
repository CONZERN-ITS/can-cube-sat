<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="9.5.2">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="24" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="88" name="SimResults" color="9" fill="1" visible="yes" active="yes"/>
<layer number="89" name="SimProbes" color="9" fill="1" visible="yes" active="yes"/>
<layer number="90" name="Modules" color="5" fill="1" visible="yes" active="yes"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="no" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="99" name="SpiceOrder" color="5" fill="1" visible="no" active="no"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="diode" urn="urn:adsk.eagle:library:210">
<description>&lt;b&gt;Diodes&lt;/b&gt;&lt;p&gt;
Based on the following sources:
&lt;ul&gt;
&lt;li&gt;Motorola : www.onsemi.com
&lt;li&gt;Fairchild : www.fairchildsemi.com
&lt;li&gt;Philips : www.semiconductors.com
&lt;li&gt;Vishay : www.vishay.de
&lt;/ul&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="SOD123" urn="urn:adsk.eagle:footprint:43203/1" library_version="8">
<description>&lt;b&gt;Diode&lt;/b&gt;</description>
<wire x1="-1.1" y1="0.7" x2="1.1" y2="0.7" width="0.254" layer="51"/>
<wire x1="1.1" y1="0.7" x2="1.1" y2="-0.7" width="0.254" layer="51"/>
<wire x1="1.1" y1="-0.7" x2="-1.1" y2="-0.7" width="0.254" layer="51"/>
<wire x1="-1.1" y1="-0.7" x2="-1.1" y2="0.7" width="0.254" layer="51"/>
<smd name="C" x="-1.9" y="0" dx="1.4" dy="1.4" layer="1"/>
<smd name="A" x="1.9" y="0" dx="1.4" dy="1.4" layer="1"/>
<text x="-1.1" y="1" size="1.27" layer="25">&gt;NAME</text>
<text x="-1.1" y="-2.3" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-1.95" y1="-0.45" x2="-1.2" y2="0.4" layer="51"/>
<rectangle x1="1.2" y1="-0.45" x2="1.95" y2="0.4" layer="51"/>
<rectangle x1="-1.05" y1="-0.65" x2="-0.15" y2="0.7" layer="51"/>
</package>
</packages>
<packages3d>
<package3d name="SOD123" urn="urn:adsk.eagle:package:43420/1" type="box" library_version="8">
<description>Diode</description>
<packageinstances>
<packageinstance name="SOD123"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="D" urn="urn:adsk.eagle:symbol:43091/2" library_version="8">
<wire x1="-1.27" y1="-1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="-1.27" y2="1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="1.27" x2="1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="1.27" x2="-1.27" y2="0" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="1.27" y2="-1.27" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-2.54" y2="0" width="0.254" layer="94"/>
<wire x1="2.54" y1="0" x2="1.27" y2="0" width="0.254" layer="94"/>
<text x="2.54" y="0.4826" size="1.778" layer="95">&gt;NAME</text>
<text x="2.54" y="-2.3114" size="1.778" layer="96">&gt;VALUE</text>
<text x="-2.54" y="0" size="0.4064" layer="99" align="center">SpiceOrder 1</text>
<text x="2.54" y="0" size="0.4064" layer="99" align="center">SpiceOrder 2</text>
<pin name="A" x="-2.54" y="0" visible="off" length="point" direction="pas"/>
<pin name="C" x="2.54" y="0" visible="off" length="point" direction="pas" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="MBR0520LT" urn="urn:adsk.eagle:component:43643/3" prefix="D" library_version="8">
<description>&lt;b&gt;SCHOTTKY BARRIER RECTIFIER&lt;/b&gt;&lt;p&gt;
Source: http://onsemi.com .. MBR0520LT1-D.pdf</description>
<gates>
<gate name="G$1" symbol="D" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOD123">
<connects>
<connect gate="G$1" pin="A" pad="A"/>
<connect gate="G$1" pin="C" pad="C"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:43420/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="POPULARITY" value="2" constant="no"/>
</technology>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="CanCubeSat">
<packages>
<package name="SOLAR_0">
<wire x1="-10" y1="-10" x2="10" y2="-10" width="0.2" layer="21"/>
<wire x1="10" y1="-10" x2="10" y2="9.5" width="0.2" layer="21"/>
<wire x1="10" y1="9.5" x2="-10" y2="9.5" width="0.2" layer="21"/>
<wire x1="-10" y1="9.5" x2="-10" y2="-10" width="0.2" layer="21"/>
<smd name="IN" x="0" y="-2.5" dx="15" dy="2" layer="1" roundness="100" rot="R90"/>
<pad name="OUT" x="0" y="9.75" drill="1" diameter="2" shape="octagon"/>
<text x="-10" y="12.3" size="1.27" layer="25">solar</text>
</package>
<package name="SOLAR_1">
<wire x1="-10" y1="-10" x2="10" y2="-10" width="0.127" layer="21"/>
<wire x1="10" y1="-10" x2="10" y2="9.5" width="0.127" layer="21"/>
<wire x1="10" y1="9.5" x2="-10" y2="9.5" width="0.127" layer="21"/>
<wire x1="-10" y1="9.5" x2="-10" y2="-10" width="0.127" layer="21"/>
<smd name="IN" x="0" y="-2.5" dx="15" dy="2" layer="1" roundness="100" rot="R90"/>
<smd name="OUT" x="0" y="2" dx="15" dy="2" layer="16" roundness="100" rot="R90"/>
<text x="-10" y="11" size="1.27" layer="25">solar</text>
</package>
</packages>
<symbols>
<symbol name="SOLAR">
<pin name="IN" x="-2.54" y="-5.08" length="middle" rot="R90"/>
<pin name="OUT" x="2.54" y="-5.08" length="middle" rot="R90"/>
<wire x1="-2.54" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<text x="-5.08" y="7.62" size="1.778" layer="95">solar</text>
</symbol>
</symbols>
<devicesets>
<deviceset name="SOLAR">
<gates>
<gate name="G$1" symbol="SOLAR" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SOLAR_0">
<connects>
<connect gate="G$1" pin="IN" pad="IN"/>
<connect gate="G$1" pin="OUT" pad="OUT"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="SOLAR_1" package="SOLAR_1">
<connects>
<connect gate="G$1" pin="IN" pad="IN"/>
<connect gate="G$1" pin="OUT" pad="OUT"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="D23" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D24" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D25" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D26" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D27" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D28" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D29" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D30" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D31" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D32" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D33" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D34" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D35" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D36" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D37" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D38" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="D1" library="diode" library_urn="urn:adsk.eagle:library:210" deviceset="MBR0520LT" device="" package3d_urn="urn:adsk.eagle:package:43420/1"/>
<part name="SOL1" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL2" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL3" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL5" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL6" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL7" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL9" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL10" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL11" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL13" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL14" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL15" library="CanCubeSat" deviceset="SOLAR" device=""/>
<part name="SOL16" library="CanCubeSat" deviceset="SOLAR" device="SOLAR_1"/>
<part name="SOL12" library="CanCubeSat" deviceset="SOLAR" device="SOLAR_1"/>
<part name="SOL8" library="CanCubeSat" deviceset="SOLAR" device="SOLAR_1"/>
<part name="SOL4" library="CanCubeSat" deviceset="SOLAR" device="SOLAR_1"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="D23" gate="G$1" x="-5.08" y="-10.16" smashed="yes">
<attribute name="NAME" x="-2.54" y="-9.6774" size="1.778" layer="95"/>
<attribute name="VALUE" x="-2.54" y="-12.4714" size="1.778" layer="96"/>
</instance>
<instance part="D24" gate="G$1" x="5.08" y="-10.16" smashed="yes">
<attribute name="NAME" x="7.62" y="-9.6774" size="1.778" layer="95"/>
<attribute name="VALUE" x="7.62" y="-12.4714" size="1.778" layer="96"/>
</instance>
<instance part="D25" gate="G$1" x="15.24" y="-10.16" smashed="yes">
<attribute name="NAME" x="17.78" y="-9.6774" size="1.778" layer="95"/>
<attribute name="VALUE" x="17.78" y="-12.4714" size="1.778" layer="96"/>
</instance>
<instance part="D26" gate="G$1" x="15.24" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="12.7" y="-5.5626" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="12.7" y="-2.7686" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D27" gate="G$1" x="5.08" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="2.54" y="-5.5626" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="2.54" y="-2.7686" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D28" gate="G$1" x="-5.08" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="-7.62" y="-5.5626" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="-7.62" y="-2.7686" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D29" gate="G$1" x="-15.24" y="-10.16" smashed="yes">
<attribute name="NAME" x="-12.7" y="-9.6774" size="1.778" layer="95"/>
<attribute name="VALUE" x="-12.7" y="-12.4714" size="1.778" layer="96"/>
</instance>
<instance part="D30" gate="G$1" x="-15.24" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="-17.78" y="-5.5626" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="-17.78" y="-2.7686" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D31" gate="G$1" x="-15.24" y="27.94" smashed="yes">
<attribute name="NAME" x="-12.7" y="28.4226" size="1.778" layer="95"/>
<attribute name="VALUE" x="-12.7" y="25.6286" size="1.778" layer="96"/>
</instance>
<instance part="D32" gate="G$1" x="-5.08" y="27.94" smashed="yes">
<attribute name="NAME" x="-2.54" y="28.4226" size="1.778" layer="95"/>
<attribute name="VALUE" x="-2.54" y="25.6286" size="1.778" layer="96"/>
</instance>
<instance part="D33" gate="G$1" x="5.08" y="27.94" smashed="yes">
<attribute name="NAME" x="7.62" y="28.4226" size="1.778" layer="95"/>
<attribute name="VALUE" x="7.62" y="25.6286" size="1.778" layer="96"/>
</instance>
<instance part="D34" gate="G$1" x="15.24" y="27.94" smashed="yes">
<attribute name="NAME" x="17.78" y="28.4226" size="1.778" layer="95"/>
<attribute name="VALUE" x="17.78" y="25.6286" size="1.778" layer="96"/>
</instance>
<instance part="D35" gate="G$1" x="15.24" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="12.7" y="32.5374" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="12.7" y="35.3314" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D36" gate="G$1" x="5.08" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="2.54" y="32.5374" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="2.54" y="35.3314" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D37" gate="G$1" x="-5.08" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="-7.62" y="32.5374" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="-7.62" y="35.3314" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D38" gate="G$1" x="-15.24" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="-17.78" y="32.5374" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="-17.78" y="35.3314" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="D1" gate="G$1" x="-22.86" y="48.26" smashed="yes" rot="R180">
<attribute name="NAME" x="-25.4" y="47.7774" size="1.778" layer="95" rot="R180"/>
<attribute name="VALUE" x="-25.4" y="50.5714" size="1.778" layer="96" rot="R180"/>
</instance>
<instance part="SOL1" gate="G$1" x="-15.24" y="-20.32" smashed="yes"/>
<instance part="SOL2" gate="G$1" x="-5.08" y="-20.32" smashed="yes"/>
<instance part="SOL3" gate="G$1" x="5.08" y="-20.32" smashed="yes"/>
<instance part="SOL5" gate="G$1" x="15.24" y="5.08" smashed="yes" rot="R180"/>
<instance part="SOL6" gate="G$1" x="5.08" y="5.08" smashed="yes" rot="R180"/>
<instance part="SOL7" gate="G$1" x="-5.08" y="5.08" smashed="yes" rot="R180"/>
<instance part="SOL9" gate="G$1" x="-15.24" y="17.78" smashed="yes"/>
<instance part="SOL10" gate="G$1" x="-5.08" y="17.78" smashed="yes"/>
<instance part="SOL11" gate="G$1" x="5.08" y="17.78" smashed="yes"/>
<instance part="SOL13" gate="G$1" x="15.24" y="43.18" smashed="yes" rot="R180"/>
<instance part="SOL14" gate="G$1" x="5.08" y="43.18" smashed="yes" rot="R180"/>
<instance part="SOL15" gate="G$1" x="-5.08" y="43.18" smashed="yes" rot="R180"/>
<instance part="SOL16" gate="G$1" x="-15.24" y="43.18" smashed="yes" rot="R180"/>
<instance part="SOL12" gate="G$1" x="15.24" y="17.78" smashed="yes"/>
<instance part="SOL8" gate="G$1" x="-15.24" y="5.08" smashed="yes" rot="R180"/>
<instance part="SOL4" gate="G$1" x="15.24" y="-20.32" smashed="yes"/>
</instances>
<busses>
</busses>
<nets>
<net name="N$1" class="0">
<segment>
<wire x1="-12.7" y1="-25.4" x2="-10.16" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="-10.16" y1="-25.4" x2="-7.62" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="-10.16" y1="-25.4" x2="-10.16" y2="-10.16" width="0.1524" layer="91"/>
<junction x="-10.16" y="-25.4"/>
<wire x1="-10.16" y1="-10.16" x2="-7.62" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D23" gate="G$1" pin="A"/>
<pinref part="SOL1" gate="G$1" pin="OUT"/>
<pinref part="SOL2" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<wire x1="7.62" y1="-25.4" x2="12.7" y2="-25.4" width="0.1524" layer="91"/>
<pinref part="SOL3" gate="G$1" pin="OUT"/>
<pinref part="SOL4" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<wire x1="2.54" y1="10.16" x2="-2.54" y2="10.16" width="0.1524" layer="91"/>
<pinref part="SOL6" gate="G$1" pin="OUT"/>
<pinref part="SOL7" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$8" class="0">
<segment>
<wire x1="-20.32" y1="12.7" x2="-20.32" y2="27.94" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="27.94" x2="-17.78" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D31" gate="G$1" pin="A"/>
<wire x1="-17.78" y1="12.7" x2="-20.32" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="-5.08" x2="-20.32" y2="10.16" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="10.16" x2="-20.32" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-17.78" y1="-5.08" x2="-20.32" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D30" gate="G$1" pin="C"/>
<junction x="-20.32" y="12.7"/>
<wire x1="-17.78" y1="10.16" x2="-20.32" y2="10.16" width="0.1524" layer="91"/>
<junction x="-20.32" y="10.16"/>
<pinref part="SOL9" gate="G$1" pin="IN"/>
<pinref part="SOL8" gate="G$1" pin="OUT"/>
</segment>
</net>
<net name="N$9" class="0">
<segment>
<wire x1="-12.7" y1="12.7" x2="-7.62" y2="12.7" width="0.1524" layer="91"/>
<pinref part="SOL9" gate="G$1" pin="OUT"/>
<pinref part="SOL10" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$10" class="0">
<segment>
<wire x1="-2.54" y1="12.7" x2="2.54" y2="12.7" width="0.1524" layer="91"/>
<pinref part="SOL10" gate="G$1" pin="OUT"/>
<pinref part="SOL11" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$11" class="0">
<segment>
<wire x1="7.62" y1="12.7" x2="12.7" y2="12.7" width="0.1524" layer="91"/>
<pinref part="SOL11" gate="G$1" pin="OUT"/>
<pinref part="SOL12" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$12" class="0">
<segment>
<wire x1="17.78" y1="12.7" x2="20.32" y2="12.7" width="0.1524" layer="91"/>
<wire x1="20.32" y1="12.7" x2="20.32" y2="48.26" width="0.1524" layer="91"/>
<wire x1="20.32" y1="48.26" x2="17.78" y2="48.26" width="0.1524" layer="91"/>
<pinref part="SOL13" gate="G$1" pin="IN"/>
<pinref part="SOL12" gate="G$1" pin="OUT"/>
</segment>
</net>
<net name="N$13" class="0">
<segment>
<wire x1="12.7" y1="48.26" x2="7.62" y2="48.26" width="0.1524" layer="91"/>
<pinref part="SOL13" gate="G$1" pin="OUT"/>
<pinref part="SOL14" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$14" class="0">
<segment>
<wire x1="2.54" y1="48.26" x2="-2.54" y2="48.26" width="0.1524" layer="91"/>
<pinref part="SOL14" gate="G$1" pin="OUT"/>
<pinref part="SOL15" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$15" class="0">
<segment>
<wire x1="-7.62" y1="48.26" x2="-12.7" y2="48.26" width="0.1524" layer="91"/>
<pinref part="SOL15" gate="G$1" pin="OUT"/>
<pinref part="SOL16" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$16" class="0">
<segment>
<wire x1="0" y1="-10.16" x2="2.54" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D24" gate="G$1" pin="A"/>
<wire x1="0" y1="-25.4" x2="2.54" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="0" y1="-10.16" x2="0" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="-2.54" y1="-10.16" x2="0" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D23" gate="G$1" pin="C"/>
<wire x1="-2.54" y1="-25.4" x2="0" y2="-25.4" width="0.1524" layer="91"/>
<junction x="0" y="-25.4"/>
<pinref part="SOL2" gate="G$1" pin="OUT"/>
<pinref part="SOL3" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$17" class="0">
<segment>
<wire x1="10.16" y1="-10.16" x2="10.16" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="7.62" y1="-10.16" x2="10.16" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D24" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$18" class="0">
<segment>
<wire x1="10.16" y1="-25.4" x2="10.16" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="10.16" y1="-10.16" x2="12.7" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D25" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$19" class="0">
<segment>
<wire x1="20.32" y1="-10.16" x2="20.32" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="17.78" y1="-10.16" x2="20.32" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D25" gate="G$1" pin="C"/>
<wire x1="17.78" y1="-25.4" x2="20.32" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="20.32" y1="-10.16" x2="20.32" y2="10.16" width="0.1524" layer="91"/>
<wire x1="17.78" y1="10.16" x2="20.32" y2="10.16" width="0.1524" layer="91"/>
<pinref part="SOL5" gate="G$1" pin="IN"/>
<pinref part="SOL4" gate="G$1" pin="OUT"/>
</segment>
</net>
<net name="N$20" class="0">
<segment>
<wire x1="20.32" y1="10.16" x2="20.32" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="20.32" y1="-5.08" x2="17.78" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D26" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$22" class="0">
<segment>
<wire x1="10.16" y1="-5.08" x2="7.62" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D27" gate="G$1" pin="A"/>
<wire x1="10.16" y1="-5.08" x2="10.16" y2="10.16" width="0.1524" layer="91"/>
<wire x1="12.7" y1="-5.08" x2="10.16" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D26" gate="G$1" pin="C"/>
<wire x1="7.62" y1="10.16" x2="10.16" y2="10.16" width="0.1524" layer="91"/>
<wire x1="12.7" y1="10.16" x2="10.16" y2="10.16" width="0.1524" layer="91"/>
<junction x="10.16" y="10.16"/>
<pinref part="SOL5" gate="G$1" pin="OUT"/>
<pinref part="SOL6" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$23" class="0">
<segment>
<wire x1="0" y1="-5.08" x2="0" y2="10.16" width="0.1524" layer="91"/>
<wire x1="2.54" y1="-5.08" x2="0" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D27" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$24" class="0">
<segment>
<wire x1="0" y1="10.16" x2="0" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="0" y1="-5.08" x2="-2.54" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D28" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$26" class="0">
<segment>
<wire x1="-20.32" y1="-25.4" x2="-20.32" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="-10.16" x2="-17.78" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D29" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$27" class="0">
<segment>
<wire x1="-10.16" y1="-10.16" x2="-10.16" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="-12.7" y1="-10.16" x2="-10.16" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D29" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$28" class="0">
<segment>
<wire x1="-10.16" y1="-5.08" x2="-12.7" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D30" gate="G$1" pin="A"/>
<wire x1="-10.16" y1="-5.08" x2="-10.16" y2="10.16" width="0.1524" layer="91"/>
<wire x1="-7.62" y1="-5.08" x2="-10.16" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D28" gate="G$1" pin="C"/>
<wire x1="-12.7" y1="10.16" x2="-10.16" y2="10.16" width="0.1524" layer="91"/>
<wire x1="-7.62" y1="10.16" x2="-10.16" y2="10.16" width="0.1524" layer="91"/>
<junction x="-10.16" y="10.16"/>
<pinref part="SOL7" gate="G$1" pin="OUT"/>
<pinref part="SOL8" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$31" class="0">
<segment>
<wire x1="-10.16" y1="27.94" x2="-10.16" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-12.7" y1="27.94" x2="-10.16" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D31" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$32" class="0">
<segment>
<wire x1="-10.16" y1="12.7" x2="-10.16" y2="27.94" width="0.1524" layer="91"/>
<wire x1="-10.16" y1="27.94" x2="-7.62" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D32" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$33" class="0">
<segment>
<wire x1="0" y1="27.94" x2="0" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-2.54" y1="27.94" x2="0" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D32" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$34" class="0">
<segment>
<wire x1="0" y1="12.7" x2="0" y2="27.94" width="0.1524" layer="91"/>
<wire x1="0" y1="27.94" x2="2.54" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D33" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$35" class="0">
<segment>
<wire x1="10.16" y1="27.94" x2="10.16" y2="12.7" width="0.1524" layer="91"/>
<wire x1="7.62" y1="27.94" x2="10.16" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D33" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$36" class="0">
<segment>
<wire x1="10.16" y1="12.7" x2="10.16" y2="27.94" width="0.1524" layer="91"/>
<wire x1="10.16" y1="27.94" x2="12.7" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D34" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$37" class="0">
<segment>
<wire x1="20.32" y1="27.94" x2="20.32" y2="12.7" width="0.1524" layer="91"/>
<wire x1="17.78" y1="27.94" x2="20.32" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D34" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$38" class="0">
<segment>
<wire x1="20.32" y1="48.26" x2="20.32" y2="33.02" width="0.1524" layer="91"/>
<wire x1="20.32" y1="33.02" x2="17.78" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D35" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$39" class="0">
<segment>
<wire x1="10.16" y1="33.02" x2="10.16" y2="48.26" width="0.1524" layer="91"/>
<wire x1="12.7" y1="33.02" x2="10.16" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D35" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$40" class="0">
<segment>
<wire x1="10.16" y1="48.26" x2="10.16" y2="33.02" width="0.1524" layer="91"/>
<wire x1="10.16" y1="33.02" x2="7.62" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D36" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$41" class="0">
<segment>
<wire x1="0" y1="33.02" x2="0" y2="48.26" width="0.1524" layer="91"/>
<wire x1="2.54" y1="33.02" x2="0" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D36" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$42" class="0">
<segment>
<wire x1="0" y1="48.26" x2="0" y2="33.02" width="0.1524" layer="91"/>
<wire x1="0" y1="33.02" x2="-2.54" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D37" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$43" class="0">
<segment>
<wire x1="-10.16" y1="33.02" x2="-10.16" y2="48.26" width="0.1524" layer="91"/>
<wire x1="-7.62" y1="33.02" x2="-10.16" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D37" gate="G$1" pin="C"/>
</segment>
</net>
<net name="N$44" class="0">
<segment>
<wire x1="-10.16" y1="48.26" x2="-10.16" y2="33.02" width="0.1524" layer="91"/>
<wire x1="-10.16" y1="33.02" x2="-12.7" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D38" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$45" class="0">
<segment>
<wire x1="-20.32" y1="33.02" x2="-20.32" y2="48.26" width="0.1524" layer="91"/>
<wire x1="-17.78" y1="33.02" x2="-20.32" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D38" gate="G$1" pin="C"/>
<pinref part="D1" gate="G$1" pin="A"/>
</segment>
</net>
<net name="N$29" class="0">
<segment>
<wire x1="-17.78" y1="-25.4" x2="-25.4" y2="-25.4" width="0.1524" layer="91"/>
<pinref part="SOL1" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$30" class="0">
<segment>
<wire x1="-17.78" y1="48.26" x2="-25.4" y2="48.26" width="0.1524" layer="91"/>
<pinref part="D1" gate="G$1" pin="C"/>
<wire x1="-25.4" y1="48.26" x2="-27.94" y2="48.26" width="0.1524" layer="91"/>
<junction x="-25.4" y="48.26"/>
<pinref part="SOL16" gate="G$1" pin="OUT"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
<compatibility>
<note version="8.2" severity="warning">
Since Version 8.2, EAGLE supports online libraries. The ids
of those online libraries will not be understood (or retained)
with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports URNs for individual library
assets (packages, symbols, and devices). The URNs of those assets
will not be understood (or retained) with this version.
</note>
<note version="8.3" severity="warning">
Since Version 8.3, EAGLE supports the association of 3D packages
with devices in libraries, schematics, and board files. Those 3D
packages will not be understood (or retained) with this version.
</note>
</compatibility>
</eagle>
