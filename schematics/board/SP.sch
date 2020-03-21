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
<layer number="100" name="DocFrame" color="15" fill="1" visible="yes" active="yes"/>
<layer number="101" name="NotChangedData" color="15" fill="1" visible="yes" active="yes"/>
<layer number="102" name="ChangedData" color="15" fill="1" visible="yes" active="yes"/>
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
<smd name="IN" x="0" y="-5" dx="10" dy="2" layer="1" roundness="100" rot="R90"/>
<pad name="OUT" x="0" y="9.75" drill="1" diameter="2" shape="octagon"/>
<text x="-10" y="12.3" size="1.27" layer="25">solar</text>
</package>
<package name="SOLAR_1">
<wire x1="-10" y1="-10" x2="10" y2="-10" width="0.127" layer="21"/>
<wire x1="10" y1="-10" x2="10" y2="9.5" width="0.127" layer="21"/>
<wire x1="10" y1="9.5" x2="-10" y2="9.5" width="0.127" layer="21"/>
<wire x1="-10" y1="9.5" x2="-10" y2="-10" width="0.127" layer="21"/>
<smd name="IN" x="0" y="-5" dx="10" dy="2" layer="1" roundness="100" rot="R90"/>
<smd name="OUT" x="0" y="4.5" dx="10" dy="2" layer="16" roundness="100" rot="R90"/>
<text x="-10" y="11" size="1.27" layer="25">solar</text>
</package>
<package name="KCONNECT">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt; - 0.1"</description>
<wire x1="-4" y1="-4.5" x2="-4" y2="4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="4.5" x2="4" y2="4.5" width="0.254" layer="21"/>
<wire x1="4" y1="4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="-4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<pad name="1" x="-3" y="-3.5" drill="1.016" shape="square"/>
<pad name="2" x="-3" y="3.5" drill="1.016" shape="octagon"/>
<pad name="3" x="3" y="-3.5" drill="1.016" shape="octagon"/>
<pad name="4" x="3" y="3.5" drill="1.016" shape="octagon"/>
<text x="-3.54" y="5.175" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-3.54" y="-6.445" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-3.254" y1="-3.754" x2="-2.746" y2="-3.246" layer="51"/>
<rectangle x1="-3.254" y1="3.246" x2="-2.746" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="3.246" x2="3.254" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="-3.754" x2="3.254" y2="-3.246" layer="51"/>
<smd name="P$2" x="0" y="0" dx="8" dy="9" layer="1"/>
<pad name="P$1" x="0" y="0" drill="6"/>
</package>
<package name="KCONNECT-">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt; - 0.1"</description>
<wire x1="-4" y1="-4.5" x2="-4" y2="4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="4.5" x2="4" y2="4.5" width="0.254" layer="21"/>
<wire x1="4" y1="4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="-4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<pad name="1" x="-3" y="-3.5" drill="1" diameter="1.4" shape="square"/>
<pad name="2" x="-3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<pad name="3" x="3" y="-3.5" drill="1" diameter="1.4" shape="octagon"/>
<pad name="4" x="3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<text x="-3.54" y="5.175" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-3.54" y="-6.445" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-3.254" y1="-3.754" x2="-2.746" y2="-3.246" layer="51"/>
<rectangle x1="-3.254" y1="3.246" x2="-2.746" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="3.246" x2="3.254" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="-3.754" x2="3.254" y2="-3.246" layer="51"/>
</package>
<package name="KCONNECT-3">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt; - 0.1"</description>
<wire x1="-4" y1="-4.5" x2="-4" y2="4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="4.5" x2="4" y2="4.5" width="0.254" layer="21"/>
<wire x1="4" y1="4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="-4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<pad name="1" x="-3" y="-3.5" drill="1" diameter="1.4" shape="square"/>
<pad name="2" x="-3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<pad name="4" x="3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<text x="-3.54" y="5.175" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-3.54" y="-6.445" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-3.254" y1="-3.754" x2="-2.746" y2="-3.246" layer="51"/>
<rectangle x1="-3.254" y1="3.246" x2="-2.746" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="3.246" x2="3.254" y2="3.754" layer="51"/>
</package>
<package name="KCONNECT-4">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt; - 0.1"</description>
<wire x1="-4" y1="-4.5" x2="-4" y2="4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="4.5" x2="4" y2="4.5" width="0.254" layer="21"/>
<wire x1="4" y1="4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="-4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<pad name="1" x="-3" y="-3.5" drill="1" diameter="1.4" shape="square"/>
<pad name="2" x="-3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<pad name="3" x="3" y="-3.5" drill="1" diameter="1.4" shape="octagon"/>
<text x="-3.54" y="5.175" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-3.54" y="-6.445" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-3.254" y1="-3.754" x2="-2.746" y2="-3.246" layer="51"/>
<rectangle x1="-3.254" y1="3.246" x2="-2.746" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="-3.754" x2="3.254" y2="-3.246" layer="51"/>
</package>
<package name="KCONNECT-1">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt; - 0.1"</description>
<wire x1="-4" y1="-4.5" x2="-4" y2="4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="4.5" x2="4" y2="4.5" width="0.254" layer="21"/>
<wire x1="4" y1="4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="-4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<pad name="2" x="-3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<pad name="3" x="3" y="-3.5" drill="1" diameter="1.4" shape="octagon"/>
<pad name="4" x="3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<text x="-3.54" y="5.175" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-3.54" y="-6.445" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-3.254" y1="3.246" x2="-2.746" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="3.246" x2="3.254" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="-3.754" x2="3.254" y2="-3.246" layer="51"/>
</package>
<package name="KCONNECT-2">
<description>&lt;b&gt;PIN HEADER&lt;/b&gt; - 0.1"</description>
<wire x1="-4" y1="-4.5" x2="-4" y2="4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="4.5" x2="4" y2="4.5" width="0.254" layer="21"/>
<wire x1="4" y1="4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<wire x1="-4" y1="-4.5" x2="4" y2="-4.5" width="0.254" layer="21"/>
<pad name="1" x="-3" y="-3.5" drill="1" diameter="1.4" shape="square"/>
<pad name="3" x="3" y="-3.5" drill="1" diameter="1.4" shape="octagon"/>
<pad name="4" x="3" y="3.5" drill="1" diameter="1.4" shape="octagon"/>
<text x="-3.54" y="5.175" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="-3.54" y="-6.445" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-3.254" y1="-3.754" x2="-2.746" y2="-3.246" layer="51"/>
<rectangle x1="2.746" y1="3.246" x2="3.254" y2="3.754" layer="51"/>
<rectangle x1="2.746" y1="-3.754" x2="3.254" y2="-3.246" layer="51"/>
</package>
<package name="255SB" urn="urn:adsk.eagle:footprint:27544/1" locally_modified="yes">
<description>&lt;b&gt;SLIDING SWITCH&lt;/b&gt;&lt;p&gt;
distributor Buerklin, 11G810</description>
<wire x1="-4.3" y1="1.75" x2="4.3" y2="1.75" width="0.1524" layer="21"/>
<wire x1="4.3" y1="-1.75" x2="4.3" y2="1.75" width="0.1524" layer="21"/>
<wire x1="4.3" y1="-1.75" x2="-4.3" y2="-1.75" width="0.1524" layer="21"/>
<wire x1="-4.3" y1="1.75" x2="-4.3" y2="-1.75" width="0.1524" layer="21"/>
<wire x1="3.81" y1="-1.397" x2="3.81" y2="1.397" width="0.1524" layer="51"/>
<wire x1="-3.81" y1="1.397" x2="-3.81" y2="-1.397" width="0.1524" layer="51"/>
<wire x1="-0.762" y1="1.27" x2="-0.762" y2="-1.27" width="0.1524" layer="51"/>
<wire x1="-1.27" y1="0.762" x2="-1.27" y2="-0.762" width="0.1524" layer="51"/>
<wire x1="3.81" y1="-1.524" x2="-0.762" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-1.778" y1="1.524" x2="-1.778" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-1.778" y1="1.524" x2="-1.27" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-1.778" y1="-1.524" x2="-2.286" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-2.286" y1="1.524" x2="-2.286" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-2.286" y1="1.524" x2="-1.778" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-2.794" y1="1.016" x2="-2.794" y2="-1.016" width="0.1524" layer="51"/>
<wire x1="-3.302" y1="1.397" x2="-3.302" y2="-1.397" width="0.1524" layer="51"/>
<wire x1="-3.81" y1="1.524" x2="-3.302" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-2.286" y1="-1.524" x2="-2.794" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="3.81" y1="-1.524" x2="3.81" y2="-1.397" width="0.1524" layer="21"/>
<wire x1="3.81" y1="1.524" x2="3.81" y2="1.397" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="-1.27" x2="-0.762" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="-1.524" x2="-1.27" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="-1.524" x2="-1.27" y2="-0.762" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="-1.524" x2="-1.778" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-2.794" y1="-1.016" x2="-2.794" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-2.794" y1="-1.524" x2="-3.302" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-3.302" y1="-1.397" x2="-3.302" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-3.302" y1="-1.524" x2="-3.81" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-3.81" y1="-1.397" x2="-3.81" y2="-1.524" width="0.1524" layer="21"/>
<wire x1="-3.81" y1="1.524" x2="-3.81" y2="1.397" width="0.1524" layer="21"/>
<wire x1="-3.302" y1="1.524" x2="-3.302" y2="1.397" width="0.1524" layer="21"/>
<wire x1="-3.302" y1="1.524" x2="-2.794" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-2.794" y1="1.524" x2="-2.794" y2="1.016" width="0.1524" layer="21"/>
<wire x1="-2.794" y1="1.524" x2="-2.286" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="1.524" x2="-1.27" y2="0.762" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="1.524" x2="-0.762" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="1.524" x2="-0.762" y2="1.27" width="0.1524" layer="21"/>
<wire x1="-0.762" y1="1.524" x2="3.81" y2="1.524" width="0.1524" layer="21"/>
<pad name="1" x="-2.5" y="0" drill="1.1176" shape="long" rot="R90"/>
<pad name="2" x="0" y="0" drill="1.1176" shape="long" rot="R90"/>
<pad name="3" x="2.5" y="0" drill="1.1176" shape="long" rot="R90"/>
<text x="-5.207" y="-2.794" size="1.27" layer="21" ratio="10">1</text>
<text x="4.191" y="-2.794" size="1.27" layer="21" ratio="10">2</text>
<text x="-5.588" y="3.556" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="0" y="3.556" size="1.27" layer="27" ratio="10">&gt;VALUE</text>
</package>
</packages>
<packages3d>
<package3d name="255SB" urn="urn:adsk.eagle:package:27679/1" type="box">
<description>SLIDING SWITCH
distributor Buerklin, 11G810</description>
<packageinstances>
<packageinstance name="255SB"/>
</packageinstances>
</package3d>
</packages3d>
<symbols>
<symbol name="SOLAR">
<pin name="IN" x="-2.54" y="-5.08" length="middle" rot="R90"/>
<pin name="OUT" x="2.54" y="-5.08" length="middle" rot="R90"/>
<wire x1="-2.54" y1="0" x2="2.54" y2="0" width="0.254" layer="94"/>
<text x="-5.08" y="7.62" size="1.778" layer="95">solar</text>
</symbol>
<symbol name="KCONNECT">
<pin name="P$1" x="-7.62" y="0" length="middle"/>
<wire x1="-2.54" y1="2.54" x2="-2.54" y2="-2.54" width="0.254" layer="94"/>
<wire x1="-2.54" y1="-2.54" x2="5.08" y2="-2.54" width="0.254" layer="94"/>
<wire x1="5.08" y1="-2.54" x2="5.08" y2="2.54" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="-2.54" y2="2.54" width="0.254" layer="94"/>
<text x="-1.778" y="3.302" size="1.27" layer="95">&gt;NAME</text>
</symbol>
<symbol name="SIS">
<wire x1="-3.81" y1="1.905" x2="-3.81" y2="0" width="0.254" layer="94"/>
<wire x1="-3.81" y1="0" x2="-1.905" y2="0" width="0.1524" layer="94"/>
<wire x1="-3.81" y1="0" x2="-3.81" y2="-1.905" width="0.254" layer="94"/>
<wire x1="-1.27" y1="0" x2="-0.762" y2="0" width="0.1524" layer="94"/>
<wire x1="0.254" y1="0" x2="0.635" y2="0" width="0.1524" layer="94"/>
<wire x1="2.54" y1="-3.175" x2="2.54" y2="-1.905" width="0.254" layer="94"/>
<wire x1="2.54" y1="-1.905" x2="0.635" y2="3.175" width="0.254" layer="94"/>
<wire x1="3.81" y1="2.54" x2="5.08" y2="2.54" width="0.254" layer="94"/>
<wire x1="5.08" y1="2.54" x2="5.08" y2="3.175" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="1.27" y2="2.54" width="0.254" layer="94"/>
<wire x1="0" y1="2.54" x2="0" y2="3.175" width="0.254" layer="94"/>
<wire x1="1.27" y1="0" x2="1.905" y2="0" width="0.1524" layer="94"/>
<wire x1="-0.762" y1="0" x2="-0.254" y2="-0.762" width="0.1524" layer="94"/>
<wire x1="-0.254" y1="-0.762" x2="0.254" y2="0" width="0.1524" layer="94"/>
<text x="-6.35" y="-1.905" size="1.778" layer="95" rot="R90">&gt;NAME</text>
<text x="-3.81" y="3.175" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="P" x="2.54" y="-5.08" visible="pad" length="short" direction="pas" rot="R90"/>
<pin name="S" x="5.08" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
<pin name="O" x="0" y="5.08" visible="pad" length="short" direction="pas" rot="R270"/>
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
<deviceset name="KCONNECT">
<gates>
<gate name="G$1" symbol="KCONNECT" x="0" y="0"/>
</gates>
<devices>
<device name="" package="KCONNECT">
<connects>
<connect gate="G$1" pin="P$1" pad="1 2 3 4 P$1 P$2"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="-" package="KCONNECT-">
<connects>
<connect gate="G$1" pin="P$1" pad="1 2 3 4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="--" package="KCONNECT-3">
<connects>
<connect gate="G$1" pin="P$1" pad="1 2 4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="4" package="KCONNECT-4">
<connects>
<connect gate="G$1" pin="P$1" pad="1 2 3"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="1" package="KCONNECT-1">
<connects>
<connect gate="G$1" pin="P$1" pad="2 3 4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
<device name="2" package="KCONNECT-2">
<connects>
<connect gate="G$1" pin="P$1" pad="1 3 4"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="255SB" prefix="S" uservalue="yes">
<description>&lt;b&gt;SLIDING SWITCH&lt;/b&gt;&lt;p&gt;
Distributor Buerklin, 11G810</description>
<gates>
<gate name="1" symbol="SIS" x="0" y="0"/>
</gates>
<devices>
<device name="" package="255SB">
<connects>
<connect gate="1" pin="O" pad="1"/>
<connect gate="1" pin="P" pad="2"/>
<connect gate="1" pin="S" pad="3"/>
</connects>
<package3dinstances>
<package3dinstance package3d_urn="urn:adsk.eagle:package:27679/1"/>
</package3dinstances>
<technologies>
<technology name="">
<attribute name="POPULARITY" value="11" constant="no"/>
</technology>
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
<part name="U$1" library="CanCubeSat" deviceset="KCONNECT" device="--" value="KCONNECT--"/>
<part name="U$2" library="CanCubeSat" deviceset="KCONNECT" device="--" value="KCONNECT--"/>
<part name="U$3" library="CanCubeSat" deviceset="KCONNECT" device="4" value="KCONNECT4"/>
<part name="U$4" library="CanCubeSat" deviceset="KCONNECT" device="4" value="KCONNECT4"/>
<part name="U$18" library="CanCubeSat" deviceset="KCONNECT" device="4" value="KCONNECT4"/>
<part name="U$19" library="CanCubeSat" deviceset="KCONNECT" device="1" value="KCONNECT1"/>
<part name="U$20" library="CanCubeSat" deviceset="KCONNECT" device="--" value="KCONNECT--"/>
<part name="U$21" library="CanCubeSat" deviceset="KCONNECT" device="--" value="KCONNECT--"/>
<part name="S1" library="CanCubeSat" deviceset="255SB" device="" package3d_urn="urn:adsk.eagle:package:27679/1"/>
</parts>
<sheets>
<sheet>
<plain>
</plain>
<instances>
<instance part="D23" gate="G$1" x="-5.08" y="-10.16" smashed="yes">
<attribute name="NAME" x="-2.54" y="-9.6774" size="1.778" layer="95"/>
</instance>
<instance part="D24" gate="G$1" x="5.08" y="-10.16" smashed="yes">
<attribute name="NAME" x="7.62" y="-9.6774" size="1.778" layer="95"/>
</instance>
<instance part="D25" gate="G$1" x="15.24" y="-10.16" smashed="yes">
<attribute name="NAME" x="17.78" y="-9.6774" size="1.778" layer="95"/>
</instance>
<instance part="D26" gate="G$1" x="15.24" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="12.7" y="-5.5626" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D27" gate="G$1" x="5.08" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="2.54" y="-5.5626" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D28" gate="G$1" x="-5.08" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="-7.62" y="-5.5626" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D29" gate="G$1" x="-15.24" y="-10.16" smashed="yes">
<attribute name="NAME" x="-12.7" y="-9.6774" size="1.778" layer="95"/>
</instance>
<instance part="D30" gate="G$1" x="-15.24" y="-5.08" smashed="yes" rot="R180">
<attribute name="NAME" x="-17.78" y="-5.5626" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D31" gate="G$1" x="-15.24" y="27.94" smashed="yes">
<attribute name="NAME" x="-12.7" y="28.4226" size="1.778" layer="95"/>
</instance>
<instance part="D32" gate="G$1" x="-5.08" y="27.94" smashed="yes">
<attribute name="NAME" x="-2.54" y="28.4226" size="1.778" layer="95"/>
</instance>
<instance part="D33" gate="G$1" x="5.08" y="27.94" smashed="yes">
<attribute name="NAME" x="7.62" y="28.4226" size="1.778" layer="95"/>
</instance>
<instance part="D34" gate="G$1" x="15.24" y="27.94" smashed="yes">
<attribute name="NAME" x="17.78" y="28.4226" size="1.778" layer="95"/>
</instance>
<instance part="D35" gate="G$1" x="15.24" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="12.7" y="32.5374" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D36" gate="G$1" x="5.08" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="2.54" y="32.5374" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D37" gate="G$1" x="-5.08" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="-7.62" y="32.5374" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D38" gate="G$1" x="-15.24" y="33.02" smashed="yes" rot="R180">
<attribute name="NAME" x="-17.78" y="32.5374" size="1.778" layer="95" rot="R180"/>
</instance>
<instance part="D1" gate="G$1" x="-22.86" y="48.26" smashed="yes" rot="R180">
<attribute name="NAME" x="-25.4" y="47.7774" size="1.778" layer="95" rot="R180"/>
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
<instance part="U$1" gate="G$1" x="27.94" y="43.18" smashed="yes" rot="R90">
<attribute name="NAME" x="24.638" y="41.402" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="U$2" gate="G$1" x="27.94" y="12.7" smashed="yes" rot="R90">
<attribute name="NAME" x="24.638" y="10.922" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="U$3" gate="G$1" x="38.1" y="43.18" smashed="yes" rot="R90">
<attribute name="NAME" x="34.798" y="41.402" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="U$4" gate="G$1" x="38.1" y="12.7" smashed="yes" rot="R90">
<attribute name="NAME" x="34.798" y="10.922" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="U$18" gate="G$1" x="48.26" y="43.18" smashed="yes" rot="R90">
<attribute name="NAME" x="44.958" y="41.402" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="U$19" gate="G$1" x="48.26" y="12.7" smashed="yes" rot="R90">
<attribute name="NAME" x="44.958" y="10.922" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="U$20" gate="G$1" x="58.42" y="43.18" smashed="yes" rot="R90">
<attribute name="NAME" x="55.118" y="41.402" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="U$21" gate="G$1" x="58.42" y="12.7" smashed="yes" rot="R90">
<attribute name="NAME" x="55.118" y="10.922" size="1.27" layer="95" rot="R90"/>
</instance>
<instance part="S1" gate="1" x="33.02" y="-17.78" smashed="yes" rot="R180">
<attribute name="NAME" x="39.37" y="-15.875" size="1.778" layer="95" rot="R270"/>
<attribute name="VALUE" x="36.83" y="-20.955" size="1.778" layer="96" rot="R270"/>
</instance>
</instances>
<busses>
</busses>
<nets>
<net name="N$1" class="0">
<segment>
<wire x1="-10.16" y1="-25.4" x2="-7.62" y2="-25.4" width="0.1524" layer="91"/>
<junction x="-10.16" y="-25.4"/>
<wire x1="-10.16" y1="-10.16" x2="-7.62" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D23" gate="G$1" pin="A"/>
<pinref part="SOL2" gate="G$1" pin="IN"/>
<wire x1="-10.16" y1="-10.16" x2="-10.16" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="-12.7" y1="-10.16" x2="-10.16" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D29" gate="G$1" pin="C"/>
<junction x="-10.16" y="-10.16"/>
</segment>
</net>
<net name="N$3" class="0">
<segment>
<pinref part="SOL4" gate="G$1" pin="IN"/>
<wire x1="10.16" y1="-25.4" x2="12.7" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="7.62" y1="-10.16" x2="10.16" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D24" gate="G$1" pin="C"/>
<wire x1="10.16" y1="-25.4" x2="10.16" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="10.16" y1="-10.16" x2="12.7" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D25" gate="G$1" pin="A"/>
<junction x="10.16" y="-10.16"/>
<junction x="10.16" y="-25.4"/>
</segment>
</net>
<net name="N$6" class="0">
<segment>
<pinref part="SOL7" gate="G$1" pin="IN"/>
<wire x1="0" y1="10.16" x2="-2.54" y2="10.16" width="0.1524" layer="91"/>
<wire x1="2.54" y1="-5.08" x2="0" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D27" gate="G$1" pin="C"/>
<wire x1="0" y1="10.16" x2="0" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="0" y1="-5.08" x2="-2.54" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D28" gate="G$1" pin="A"/>
<junction x="0" y="10.16"/>
<junction x="0" y="-5.08"/>
</segment>
</net>
<net name="N$8" class="0">
<segment>
<wire x1="-20.32" y1="12.7" x2="-20.32" y2="27.94" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="27.94" x2="-17.78" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D31" gate="G$1" pin="A"/>
<wire x1="-17.78" y1="12.7" x2="-20.32" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="-5.08" x2="-20.32" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-17.78" y1="-5.08" x2="-20.32" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D30" gate="G$1" pin="C"/>
<junction x="-20.32" y="12.7"/>
<pinref part="SOL9" gate="G$1" pin="IN"/>
</segment>
</net>
<net name="N$9" class="0">
<segment>
<pinref part="SOL10" gate="G$1" pin="IN"/>
<wire x1="-10.16" y1="12.7" x2="-7.62" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-12.7" y1="27.94" x2="-10.16" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D31" gate="G$1" pin="C"/>
<wire x1="-10.16" y1="12.7" x2="-10.16" y2="27.94" width="0.1524" layer="91"/>
<wire x1="-10.16" y1="27.94" x2="-7.62" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D32" gate="G$1" pin="A"/>
<junction x="-10.16" y="12.7"/>
<junction x="-10.16" y="27.94"/>
</segment>
</net>
<net name="N$10" class="0">
<segment>
<pinref part="SOL11" gate="G$1" pin="IN"/>
<wire x1="0" y1="12.7" x2="2.54" y2="12.7" width="0.1524" layer="91"/>
<wire x1="-2.54" y1="27.94" x2="0" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D32" gate="G$1" pin="C"/>
<wire x1="0" y1="12.7" x2="0" y2="27.94" width="0.1524" layer="91"/>
<wire x1="0" y1="27.94" x2="2.54" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D33" gate="G$1" pin="A"/>
<junction x="0" y="12.7"/>
<junction x="0" y="27.94"/>
</segment>
</net>
<net name="N$11" class="0">
<segment>
<pinref part="SOL12" gate="G$1" pin="IN"/>
<wire x1="10.16" y1="12.7" x2="12.7" y2="12.7" width="0.1524" layer="91"/>
<wire x1="7.62" y1="27.94" x2="10.16" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D33" gate="G$1" pin="C"/>
<wire x1="10.16" y1="12.7" x2="10.16" y2="27.94" width="0.1524" layer="91"/>
<wire x1="10.16" y1="27.94" x2="12.7" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D34" gate="G$1" pin="A"/>
<junction x="10.16" y="12.7"/>
<junction x="10.16" y="27.94"/>
</segment>
</net>
<net name="N$12" class="0">
<segment>
<wire x1="20.32" y1="27.94" x2="20.32" y2="33.02" width="0.1524" layer="91"/>
<wire x1="20.32" y1="48.26" x2="17.78" y2="48.26" width="0.1524" layer="91"/>
<pinref part="SOL13" gate="G$1" pin="IN"/>
<wire x1="20.32" y1="27.94" x2="20.32" y2="12.7" width="0.1524" layer="91"/>
<wire x1="17.78" y1="27.94" x2="20.32" y2="27.94" width="0.1524" layer="91"/>
<pinref part="D34" gate="G$1" pin="C"/>
<junction x="20.32" y="27.94"/>
<wire x1="20.32" y1="48.26" x2="20.32" y2="33.02" width="0.1524" layer="91"/>
<wire x1="20.32" y1="33.02" x2="17.78" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D35" gate="G$1" pin="A"/>
<junction x="20.32" y="33.02"/>
</segment>
</net>
<net name="N$13" class="0">
<segment>
<pinref part="SOL14" gate="G$1" pin="IN"/>
<wire x1="10.16" y1="48.26" x2="7.62" y2="48.26" width="0.1524" layer="91"/>
<wire x1="12.7" y1="33.02" x2="10.16" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D35" gate="G$1" pin="C"/>
<wire x1="10.16" y1="48.26" x2="10.16" y2="33.02" width="0.1524" layer="91"/>
<wire x1="10.16" y1="33.02" x2="7.62" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D36" gate="G$1" pin="A"/>
<junction x="10.16" y="33.02"/>
<junction x="10.16" y="48.26"/>
</segment>
</net>
<net name="N$14" class="0">
<segment>
<pinref part="SOL15" gate="G$1" pin="IN"/>
<wire x1="0" y1="48.26" x2="-2.54" y2="48.26" width="0.1524" layer="91"/>
<wire x1="2.54" y1="33.02" x2="0" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D36" gate="G$1" pin="C"/>
<wire x1="0" y1="48.26" x2="0" y2="33.02" width="0.1524" layer="91"/>
<wire x1="0" y1="33.02" x2="-2.54" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D37" gate="G$1" pin="A"/>
<junction x="0" y="33.02"/>
<junction x="0" y="48.26"/>
</segment>
</net>
<net name="N$15" class="0">
<segment>
<pinref part="SOL16" gate="G$1" pin="IN"/>
<wire x1="-10.16" y1="48.26" x2="-12.7" y2="48.26" width="0.1524" layer="91"/>
<wire x1="-7.62" y1="33.02" x2="-10.16" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D37" gate="G$1" pin="C"/>
<wire x1="-10.16" y1="48.26" x2="-10.16" y2="33.02" width="0.1524" layer="91"/>
<wire x1="-10.16" y1="33.02" x2="-12.7" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D38" gate="G$1" pin="A"/>
<junction x="-10.16" y="33.02"/>
<junction x="-10.16" y="48.26"/>
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
<pinref part="SOL3" gate="G$1" pin="IN"/>
<junction x="0" y="-10.16"/>
</segment>
</net>
<net name="N$19" class="0">
<segment>
<wire x1="20.32" y1="-10.16" x2="20.32" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="17.78" y1="-10.16" x2="20.32" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D25" gate="G$1" pin="C"/>
<wire x1="20.32" y1="-10.16" x2="20.32" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="17.78" y1="10.16" x2="20.32" y2="10.16" width="0.1524" layer="91"/>
<pinref part="SOL5" gate="G$1" pin="IN"/>
<junction x="17.78" y="-10.16"/>
<junction x="20.32" y="-10.16"/>
<wire x1="20.32" y1="10.16" x2="20.32" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="20.32" y1="-5.08" x2="17.78" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="D26" gate="G$1" pin="A"/>
<junction x="20.32" y="-5.08"/>
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
<pinref part="SOL6" gate="G$1" pin="IN"/>
<junction x="10.16" y="-5.08"/>
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
<pinref part="SOL8" gate="G$1" pin="IN"/>
<junction x="-10.16" y="-5.08"/>
</segment>
</net>
<net name="BUS+1" class="0">
<segment>
<pinref part="U$1" gate="G$1" pin="P$1"/>
<wire x1="27.94" y1="35.56" x2="27.94" y2="25.4" width="0.1524" layer="91"/>
<label x="27.94" y="25.4" size="1.778" layer="95" rot="R90"/>
<pinref part="U$3" gate="G$1" pin="P$1"/>
<wire x1="38.1" y1="35.56" x2="38.1" y2="25.4" width="0.1524" layer="91"/>
<label x="38.1" y="25.4" size="1.778" layer="95" rot="R90"/>
<wire x1="38.1" y1="25.4" x2="33.02" y2="25.4" width="0.1524" layer="91"/>
<wire x1="33.02" y1="25.4" x2="33.02" y2="50.8" width="0.1524" layer="91"/>
<wire x1="33.02" y1="50.8" x2="-27.94" y2="50.8" width="0.1524" layer="91"/>
<pinref part="D1" gate="G$1" pin="C"/>
<wire x1="-20.32" y1="48.26" x2="-25.4" y2="48.26" width="0.1524" layer="91"/>
<wire x1="-25.4" y1="48.26" x2="-27.94" y2="48.26" width="0.1524" layer="91"/>
<junction x="-25.4" y="48.26"/>
<wire x1="-27.94" y1="50.8" x2="-27.94" y2="48.26" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="33.02" x2="-20.32" y2="48.26" width="0.1524" layer="91"/>
<wire x1="-17.78" y1="33.02" x2="-20.32" y2="33.02" width="0.1524" layer="91"/>
<pinref part="D38" gate="G$1" pin="C"/>
<pinref part="D1" gate="G$1" pin="A"/>
<junction x="-20.32" y="48.26"/>
<wire x1="27.94" y1="25.4" x2="33.02" y2="25.4" width="0.1524" layer="91"/>
<junction x="33.02" y="25.4"/>
<pinref part="U$20" gate="G$1" pin="P$1"/>
<wire x1="58.42" y1="35.56" x2="58.42" y2="25.4" width="0.1524" layer="91"/>
<label x="58.42" y="25.4" size="1.778" layer="95" rot="R90"/>
<wire x1="38.1" y1="25.4" x2="48.26" y2="25.4" width="0.1524" layer="91"/>
<junction x="38.1" y="25.4"/>
<pinref part="U$18" gate="G$1" pin="P$1"/>
<wire x1="48.26" y1="25.4" x2="58.42" y2="25.4" width="0.1524" layer="91"/>
<wire x1="48.26" y1="35.56" x2="48.26" y2="25.4" width="0.1524" layer="91"/>
<label x="48.26" y="25.4" size="1.778" layer="95" rot="R90"/>
<junction x="48.26" y="25.4"/>
</segment>
</net>
<net name="BUS-1" class="0">
<segment>
<wire x1="-20.32" y1="-25.4" x2="-20.32" y2="-10.16" width="0.1524" layer="91"/>
<wire x1="-20.32" y1="-10.16" x2="-17.78" y2="-10.16" width="0.1524" layer="91"/>
<pinref part="D29" gate="G$1" pin="A"/>
<wire x1="-17.78" y1="-25.4" x2="-20.32" y2="-25.4" width="0.1524" layer="91"/>
<pinref part="SOL1" gate="G$1" pin="IN"/>
<junction x="-20.32" y="-25.4"/>
<wire x1="-20.32" y1="-25.4" x2="-25.4" y2="-25.4" width="0.1524" layer="91"/>
<wire x1="27.94" y1="-22.86" x2="27.94" y2="-27.94" width="0.1524" layer="91"/>
<wire x1="27.94" y1="-27.94" x2="-25.4" y2="-27.94" width="0.1524" layer="91"/>
<wire x1="-25.4" y1="-27.94" x2="-25.4" y2="-25.4" width="0.1524" layer="91"/>
<pinref part="S1" gate="1" pin="S"/>
</segment>
</net>
<net name="N$2" class="0">
<segment>
<pinref part="U$2" gate="G$1" pin="P$1"/>
<wire x1="27.94" y1="5.08" x2="27.94" y2="-5.08" width="0.1524" layer="91"/>
<junction x="27.94" y="-5.08"/>
<pinref part="U$4" gate="G$1" pin="P$1"/>
<wire x1="38.1" y1="5.08" x2="38.1" y2="-5.08" width="0.1524" layer="91"/>
<junction x="38.1" y="-5.08"/>
<pinref part="U$21" gate="G$1" pin="P$1"/>
<wire x1="58.42" y1="5.08" x2="58.42" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="58.42" y1="-5.08" x2="48.26" y2="-5.08" width="0.1524" layer="91"/>
<pinref part="U$19" gate="G$1" pin="P$1"/>
<wire x1="48.26" y1="5.08" x2="48.26" y2="-5.08" width="0.1524" layer="91"/>
<junction x="48.26" y="-5.08"/>
<wire x1="48.26" y1="-5.08" x2="38.1" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="38.1" y1="-5.08" x2="27.94" y2="-5.08" width="0.1524" layer="91"/>
<wire x1="27.94" y1="-5.08" x2="27.94" y2="-12.7" width="0.1524" layer="91"/>
<wire x1="30.48" y1="-12.7" x2="27.94" y2="-12.7" width="0.1524" layer="91"/>
<label x="38.1" y="-5.08" size="1.778" layer="95" rot="R90"/>
<label x="58.42" y="-5.08" size="1.778" layer="95" rot="R90"/>
<label x="27.94" y="-5.08" size="1.778" layer="95" rot="R90"/>
<label x="48.26" y="-5.08" size="1.778" layer="95" rot="R90"/>
<pinref part="S1" gate="1" pin="P"/>
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
