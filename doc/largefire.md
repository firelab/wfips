<link href="wfips.css" rel="stylesheet" type="text/css">

<head><title>WFIPS: Large Fire Simulation</title></head>

Large Fire Simulation
=====================

Escape fires or subsets of escaped fires can be simulated in the Large Fire
Analyst. The Large Fire Analyst uses the escape fire's location and date to
determine the final fire size, fire cost, and population affected. Which 
escape fires are included in the large fire analysis can be specified.


Types of Escapes in WFIPS
-------------------------

Fires are either contained or classified as escapes.  Fires can escape for four
reasons:

- No Resources Sent
- Time Limit Exceeded
- Size Limit Exceeded
- Resources Exhausted

"No Resources Sent" escapes occur when no resources are sent to a fire because 
none of the resources in the dispatch logic are available.

"Time Limit Exceeded" and "Size Limit Exceeded" escapes occur when the time
limit or size limit set for the location are exceeded during the containment
simulation.

"Resources Exhausted" escapes occur when the resources that initially respond 
to the fire all time out for the day and no resources are left to contain 
the fire.

Selection Options
------------------

The percentage of each type of escape fire that will be sent to the Large 
Fire Analyst can be set and/or the start and end dates for escape fires 
to be sent to the Large Fire Analyst can be specified.  

