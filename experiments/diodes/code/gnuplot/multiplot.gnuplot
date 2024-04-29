set terminal qt size 900, 828 font "Helvetica, 24"

load "variables.txt"

data_dir = "cooked_data"

# The dot operator concatenates strings in gnmuplot.

plot_title = device_type . " at " . wavelength \
  . " nm, axis: " . orientation

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
  set palette rgb 10,13,33
  unset key
  unset colorbox
  unset border
  unset xtics
  unset ytics
  set xlabel sprintf("%.1f V, %s", voltage, resistance)

  # set cbrange [0:voltage] # If you do this, the background will always be blue.
  set cbrange [0:5.0]

  if (orientation eq "parallel") {
    plot parallel_data using 1:2:3 with image, \
      parallel_contours using 1:2 with lines lw 3 lc black
  }

  if (orientation eq "perpendicular") {
    plot perpendicular_data using 1:2:3 with image, \
      perpendicular_contours using 1:2 with lines lw 3 lc black
  }

  if (orientation eq "diagonal") {
    plot diagonal_data using 1:2:3 with image, \
      diagonal_contours using 1:2 with lines lw 3 lc black
  }
}

unset multiplot

set print "-"
print \
"Note: messages about \"Skipping data file with no valid points\" are harmless."

