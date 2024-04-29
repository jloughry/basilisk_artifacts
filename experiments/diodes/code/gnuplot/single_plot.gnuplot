set terminal qt size 900, 828 font "Helvetica, 24"

load "variables.txt"

contour_thickness = 5

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
set cbrange [0:voltage]
set palette rgb 10,13,33
unset key
set xlabel "(mm)"
set ylabel "(mm)" offset 2
set xtics -.5, .25, .5
set ytics -.5, .25, .5
set title plot_title

set label sprintf("V_{IH} = %.1f V", logic_threshold) at graph -.1,-.08 front

if (orientation eq "parallel") {
  set label "Elliptical Axis: Parallel" at graph .75,-.08

  plot parallel_data using 1:2:3 with image, \
    parallel_contours using 1:2 with lines lw contour_thickness lc black
}

if (orientation eq "perpendicular") {
  set label "Elliptical Axis: Perpendicular" at graph .65,-.08

  plot perpendicular_data using 1:2:3 with image, \
    perpendicular_contours using 1:2 with lines lw contour_thickness lc black
}

if (orientation eq "diagonal") {
  set label "Elliptical Axis: Diagonal" at graph .75,-.08

  plot diagonal_data using 1:2:3 with image, \
    diagonal_contours using 1:2 with lines lw contour_thickness lc black
}

