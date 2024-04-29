set terminal qt size 900, 828 font "Helvetica, 24"

load "variables.txt"

data_dir = "cooked_data"

# The dot operator concatenates strings in gnmuplot.

plot_title = device_type . " at " . wavelength . " nm, " \
  . sprintf("%.1f V",voltage) . ", " . resistance

parallel_data = data_dir . "/temp_file_" . wavelength . \
  "_parallel_data_" . run_number . ".txt"
parallel_contours = data_dir . "/" . wavelength . \
  "_parallel_" . run_number . "_contours.dat"

perpendicular_data = data_dir . "/temp_file_" . wavelength \
  . "_perpendicular_data_" . run_number . ".txt"
perpendicular_contours = data_dir . "/" . wavelength \
  . "_perpendicular_" . run_number . "_contours.dat"

diagonal_data = data_dir . "/temp_file_" . wavelength . \
  "_diagonal_data_" . run_number . ".txt"
diagonal_contours = data_dir . "/" . wavelength . \
  "_diagonal_" . run_number . "_contours.dat"

# set print "-"
# print parallel_data
# print parallel_contours
# print perpendicular_data
# print perpendicular_contours
# print diagonal_data
# print diagonal_contours

elevation_parallel = 0
elevation_diagonal = 1
elevation_perpendicular = 2

set dgrid3d 50,50
set contour base
set view 0,0
unset surface
set cntrparam cubicspline levels discrete logic_threshold
set table parallel_contours
splot parallel_data with lines notitle
unset table

reset
set dgrid3d 50,50
set contour base
set view 0,0 
unset surface
set cntrparam cubicspline levels discrete logic_threshold
set table perpendicular_contours
splot perpendicular_data with lines notitle
unset table

reset
set dgrid3d 50,50
set contour base
set view 0,0 
unset surface
set cntrparam cubicspline levels discrete logic_threshold
set table diagonal_contours
splot diagonal_data with lines notitle
unset table

reset
set xrange [-0.5:0.5]
set yrange [-0.5:0.5]
set zrange [0:2]
set cbrange [0:voltage]
set palette rgb 10,13,33
unset key
set xyplane 0.05
set xlabel "(mm)" offset 1.25, -.75, 0
set ylabel "(mm)" offset -2, 0, 0
set xtics -.5, .1, .5 offset -1, -.5, 0
set ytics -.5, .25, .5
set ztics ("Parallel" elevation_parallel, "Diagonal" elevation_diagonal, \
  "Perpen-\n dicular" elevation_perpendicular)
set view 69,17
set colorbox vertical user origin .85, .4 size .05,.5
set title plot_title

splot \
  parallel_data using 1:2:(elevation_parallel):3 with image, \
  parallel_contours using 1:2:(elevation_parallel) with lines lw 2 lc black, \
  diagonal_data using 1:2:(elevation_diagonal):3 with image, \
  diagonal_contours using 1:2:(elevation_diagonal) with lines lw 2 lc black, \
  perpendicular_data using 1:2:(elevation_perpendicular):3 with image, \
  perpendicular_contours using 1:2:(elevation_perpendicular) \
    with lines lw 2 lc black

