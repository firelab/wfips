<link href="wfips.css" rel="stylesheet" type="text/css">

<head><title>WFIPS: Analysis Area</title></head>

Analysis Area
=============

The analysis area selection allows you to run WFIPS on an arbitrary land mass.
The land base is defined by user-selected features from provided layers, or
from user provided layers.  The features need not be contiguous.cp

Default Analysis Area Layers
----------------------------

WFIPS comes with a default set of layers to choose popular analysis areas.  The
layers are typical administration boundaries for some government agencies:

- GACC
- USFS Forests
- USFS Districts
- US States
- US Counties
- Legacy FPU boundaries

Custom Analysis Area
--------------------

A custom analysis layer may be loaded by clicking the add layer button.  The
layer can be any geographic polygon based layer that intersects the contiguous
US.

Buffering the Analysis Feature
------------------------------

The user may choose to buffer the selected feature(s) for the analysis area.
This allows inclusion of ignitions just outside the selected feature(s).

Data Selection
--------------

After a user selects features for the analysis area, data is loaded from the
database.  The data loaded may depend on the area in different ways.

- FWA: Any FWA *intersecting* the analysis feature(s) are included.
- Dispatch Locations: Any location associated with the included FWA set will be
  included.
- Resources: If loaded from the default resource database, all resources that
  reside at any location are loaded.  These may be edited.
- Fire Ignitions: all ignitions *completely contained* inside the selected
  feature(s) (and optional buffer) will be loaded.

Map Tools
---------

- ![Pan Tool](mActionPan.png) Pan the map
- ![Zoom In Tool](mActionZoomIn.png) Zoom in on the map
- ![Zoom Out Tool](mActionZoomOut.png) Zoom out on the map
- ![Identify Tool](mActionIdentify.png) Identify feature(s) on the current layer
- ![Select Tool](mActionSelect.png) Select feature(s) for the analysis area
  from the current layer.

