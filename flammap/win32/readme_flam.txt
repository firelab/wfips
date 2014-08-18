FlamMap: command-line version

all files need to be compiled as a project

Right now, the rudimentary commandline interface is set up to
produce the following ascii files:

*.mdr	-- maximum spread direction file
*.eda	-- elliptical dimension A
*.edb	-- elliptical dimension B
*.edc	-- elliptical dimension C
*.ros	-- max spread rate (m/min)
*.fli	-- fireline intensity (kW/m)
*.cfr	-- crown fire state (0, 1, 2, 3)

You will need the .mdr, .eda, .edb, and .edc files for the fire spread model
