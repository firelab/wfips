<link href="wfips.css" rel="stylesheet" type="text/css">

<head><title>WFIPS: Management Objectives</title></head>

Management Objectives
==============

Simulate fire management objects in WFIPs. Simulated modified responses 
to fire based on the strategic response objective for the location 
of the fire. WFIPs assumes a full suppression response using FPA's 
dispatch logic to determine the numbers and types of resources deployed 
to a fire. This can be modified to allow a monitor fire response for 
locations where the Fire Management Plan allows such a response, or 
in spatially defined locations. Fire containment is not modeled 
for Fires recieving a monitor response, and monitored fires are not 
sent to large fire. 


Selection Options
---------------------------------------------- 

There are three ways to define fire managements objectives in WFIPs.

- Suppress All Fires
- Fire Management Plan Based Response
- Spatially Defined Response

"Suppress All Fires" all fires are suppressed using the dispatch
logic from FPA.

"Fire Management Plan Based Response" uses the gridded Spatial
FMU Strategic Response for Natural Fires layer to assign fires
a monitor response. Fires categorized as "fire for resource benifit"
and "preplaned agency administrator response to monitor all fires" 
recieve a minimal response rather than the response determined by 
FPA's dispatch logic.

"Spatially Defined Response" uses a geography polygon based layer to 
define the location of fires that will recieve a monitor response. 
Specifying the file to load the layer. All fires located within a 
polygon are assigned a monitor response.
 


  

