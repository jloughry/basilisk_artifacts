set terminal qt size 900, 828 font "Helvetica, 24"

load "variables.txt"

contour_thickness = 5

data_dir = "cooked_data"

# The dot operator concatenates strings in gnmuplot.

plot_title = LED_color . " LED, " . laser_wavelength . " nm laser, " \
  . sprintf("%.1f V logic",voltage) . ", " . resistance

data = data_dir . "/cooked_" . tag . "_data.txt"
contours = data_dir . "/" . LED_color . "_contours.dat"

set dgrid3d 50,50
set contour base
set view 0,0
unset surface
set cntrparam cubicspline levels discrete logic_threshold
set table contours
splot data with lines notitle
unset table

set xrange [-2.5:2.5]
set yrange [-2.5:2.5]
# set cbrange [0:voltage]
set cbrange [0:5.0]
set palette rgb 10,13,33
unset key
set xlabel "(mm)"
set ylabel "(mm)" offset 2
set xtics -2.5, 1, 2.5
set ytics -2.5, 1, 2.5
set title plot_title

set label sprintf("V_{IH} = %.1f V", logic_threshold) at graph -.1,-.08 front

plot data using 1:2:3 with image, \
    contours using 1:2 with lines lw contour_thickness lc black

