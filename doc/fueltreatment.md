<link href="wfips.css" rel="stylesheet" type="text/css">

<head><title>WFIPS: Fuel Treatments</title></head>

Fuel Treatment
==============

Simulate Hazardous Fuel Treatments. Simulated fire behavior for treated fires 
are derived from a spatial fuels layer that represents a durable (5 year) 
fuel treatment for all areas of the Western United States. If a durable 
treatment does not exist for the fuel type (i.e., grass), the untreated 
fuels are used in the layer. 

Selection Options
---------------------------------------------- 

There are three ways to define fuel treatments in WFIPs.

- No Fuel Treatment
- Random Treatment
- Risk Potential Based Treatment
- Spatial Fuel Treatment

"No Fuel Treatment" all fires are assigned fire behavior using
untreated fuels.

"Random Treatment" randomly applies treatments to simulated fires. 
The percentage of fires that are assigned fire behavior using 
treated fuels is specified. Treated fires will only have modified
fire behavior if they are in a fuel type with a durable 
treatment.

"Risk Potential Based Treatment" uses the gridded Risk Potential 
layer to determine where treated fires are located. The percentage 
of fires that will be simulated using fire behavior from the 
treated fuels layer is specified for each of the five risk potential 
levels. Treatments are randomly applied to fires based on 
these percentages. Treated fires will only have modified fire 
behavior if they are in a fuel type that has a durable treatment.

"Spatial Fuel Treatment" uses a layer of polygons to define the
location of fuel treatments. The file and layer are input. All
fires located within a ploygon are assigned fire behavior using 
the treated fuels layer. Treated fires will only have modified
fire behavior if they are in a fuel type that has a durable 
treatment.
 


  

