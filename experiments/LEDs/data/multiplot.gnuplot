set terminal qt size 900, 828 font "Helvetica, 24"

load "variables.txt"

data_dir = "cooked_data"

# The dot operator concatenates strings in gnmuplot.

plot_title = device_type . " at " . wavelength \
  . " nm"

set multiplot layout 4,4 title plot_title

do for [run_number = 1:16] {

  if (run_number >= 1 && run_number <= 4) {
    voltage = 5
    logic_threshold = 2.0
  }

  if (run_number >= 5 && run_number <= 8) {
    voltage = 3.3
    logic_threshold = 2.0
  }

  if (run_number >= 9 && run_number <= 12) {
    voltage = 2.5
    logic_threshold = 1.7
  }

  if (run_number >= 13 && run_number <= 16) {
    voltage = 1.8
    logic_threshold = 1.17
  }

  if (run_number == 1 || run_number == 5 || run_number == 9 || run_number == 13) {
    resistance = "1kΩ"
  }

  if (run_number == 2 || run_number == 6 || run_number == 10 || run_number == 14) {
    resistance = "2.2kΩ"
  }

  if (run_number == 3 || run_number == 7 || run_number == 11 || run_number == 15) {
    resistance = "4.7kΩ"
  }

  if (run_number == 4 || run_number == 8 || run_number == 12 || run_number == 16) {
    resistance = "10kΩ"
  }

  data = data_dir . "/temp_file_" . wavelength . \
    "_data_" . run_number . ".txt"
  contours = data_dir . "/" . wavelength . \
    "_" . run_number . "_contours.dat"

  set dgrid3d 50,50
  set contour base
  set view 0,0
  unset surface
  set cntrparam cubicspline levels discrete logic_threshold
  set table contours
  splot data with lines notitle
  unset table

  reset
  set xrange [-0.5:0.5]
  set yrange [-0.5:0.5]
  set palette rgb 10,13,33
  unset key
  unset colorbox
  unset border
  unset xtics
  unset ytics
  set xlabel sprintf("%.1f V, %s", voltage, resistance)

  set cbrange [0:voltage]

  plot data using 1:2:3 with image, \
      contours using 1:2 with lines lw 3 lc black

unset multiplot

set print "-"
print \
"Note: messages about \"Skipping data file with no valid points\" are harmless."

