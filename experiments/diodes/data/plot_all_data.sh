#!/bin/sh

# This script plots raw data from the experiment using Gnuplot.

# First, we specify the location of the raw data files:

raw_data_dir=raw_data
cooked_data_dir=cooked_data

data_980_parallel="20230726.2022_980_parallel.txt"
data_980_diagonal="20230727.0720_980_nm_diagonal.txt"
data_980_perpendicular="20230726.1637_980_perpendicular.txt"

data_808_parallel="20230728.1151_808_nm_diagonal.txt"
data_808_diagonal="20230729.0645_808_nm_parallel.txt"
data_808_perpendicular="20230728.2200_808_nm_perpendicular.txt"

data_780_parallel="20230728.0632_780_parallel.txt"
data_780_diagonal="20230727.1347_780_nm_diagonal.txt"
data_780_perpendicular="20230727.1802_780_nm_perpendicular.txt"

data_650_parallel="20230730.0848_650_nm_parallel.txt"
data_650_diagonal="20230729.1454_650_nm_diagonal.txt"
data_650_perpendicular="20230729.1956_650_nm_perpendicular.txt"

data_532_parallel="20230801.0652_532_nm_parallel.txt"
data_532_diagonal="20230731.1515_532_nm_diagonal.txt"
data_532_perpendicular="20230731.1855_532_nm_perpendicular.txt"

data_405_parallel="20230731.0727_405_nm_parallel.txt"
data_405_diagonal="20230730.2039_405_nm_diagonal.txt"
data_405_perpendicular="20230731.1134_405_nm_perpendicular.txt"

# Usage: verify_data_file_exists filename

function verify_data_file_exists () {
  if [ ! -f ${raw_data_dir}/$1 ]; then
    echo "missing data file: $1"
    exit 1
  fi
}

# Make sure all the raw data exists.

for wavelength in 980 808 780 650 532 405
do
  for orientation in parallel perpendicular diagonal
  do
    filename=data_${wavelength}_${orientation}
    verify_data_file_exists ${!filename}
  done
done

# Secondly, we transform the raw data into a form Gnuplot can use:

mkdir -p ${cooked_data_dir}

rm -f ${cooked_data_dir}/temp_file_*.txt
rm -f ${cooked_data_dir}/temp_file_*.txt
rm -f ${cooked_data_dir}/temp_file_*.txt
rm -rf ${cooked_data_dir}/*contours.dat

# Usage: cook wavelength orientation

function cook () {
  for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
  do
    filename=data_${1}_${2}
    # in bash, ! means indirect evaluation:
    grep "^${i}," ${raw_data_dir}/${!filename} \
      | cut -d ',' -f 5-7 \
      | sed -e 's/,/ /g' \
      > ${cooked_data_dir}/temp_file_${1}_${2}_data_${i}.txt
  done
}

for wavelength in 980 808 780 650 532 405
do
  for orientation in parallel perpendicular diagonal
  do
    cook ${wavelength} ${orientation}
  done
done

# Finally, we call Gnuplot to make various kinds of plots:

variables_file=variables.txt

# Parameters are: wavelength, voltage, resistance, logic_threshold, run_number,
# orientation

function single_plot () {
  rm -f ${variables_file}

  echo "device_type = \"1N34A (Schottky silicon equivalent) DO-35 glass\"" \
    >> ${variables_file}

  echo "wavelength =" $1 "# nanometres" >> ${variables_file}
  echo "voltage =" $2 "# volts" >> ${variables_file}
  echo "resistance = \""$3 "pull-up\" # pull-up value in ohms" \
    >> ${variables_file}
  echo "logic_threshold =" $4 "# volts" >> ${variables_file}
  echo "run_number =" $5 "# run number to plot" >> ${variables_file}
  echo "orientation =" \"$6\" "# parallel, perpendicular, or diagonal" \
    >> ${variables_file}

  gnuplot -p single_plot.gnuplot
}

# Parameters are: wavelength, voltage, resistance, logic_threshold, run_number

function stacked_plot () {
  rm -f ${variables_file}

  echo "device_type = \"1N34A (Schottky silicon equivalent) DO-35 glass\"" \
    >> ${variables_file}

  echo "wavelength =" $1 "# nanometres" >> ${variables_file}
  echo "voltage =" $2 "# volts" >> ${variables_file}
  echo "resistance = \""$3 "pull-up\" # pull-up value in ohms" \
    >> ${variables_file}
  echo "logic_threshold =" $4 "# volts" >> ${variables_file}
  echo "run_number =" $5 "# run number to plot" >> ${variables_file}

  gnuplot -p stacked_plot.gnuplot
}

# Parameters are: wavelength, orientation

function multiplot () {
  rm -f ${variables_file}

  echo "device_type = \"1N34A (Schottky silicon equivalent) DO-35 glass\"" \
    >> ${variables_file}

  echo "wavelength =" $1 "# nanometres" >> ${variables_file}
  echo "orientation =" \"$2\" "# parallel, perpendicular, or diagonal" \
    >> ${variables_file}

  gnuplot -p multiplot.gnuplot
}

# Use these for an overview:

for wavelength in 980 808 780 650 532 405
do
  for orientation in parallel perpendicular diagonal
  do
    multiplot ${wavelength} ${orientation}
  done
done

# Use these to look at individual plots in detail:

# single_plot 980 3.3 "10 kΩ" 2.0 8 parallel
# single_plot 980 3.3 "10 kΩ" 2.0 8 perpendicular
# single_plot 980 3.3 "10 kΩ" 2.0 8 diagonal

# Use these to look at the effect of elliptical beam axis orientation

# stacked_plot 780 5 "10kΩ" 2.0 4
# stacked_plot 780 3.3 "10kΩ" 2.0 8
# stacked_plot 780 2.5 "10kΩ" 1.7 12
# stacked_plot 780 1.8 "10kΩ" 1.17 16

# stacked_plot 808 5 "10kΩ" 2.0 4
# stacked_plot 808 3.3 "10kΩ" 2.0 8
# stacked_plot 808 2.5 "10kΩ" 1.7 12
# stacked_plot 808 1.8 "10kΩ" 1.17 16

# stacked_plot 980 5 "10kΩ" 2.0 4
# stacked_plot 980 3.3 "10kΩ" 2.0 8
# stacked_plot 980 2.5 "10kΩ" 1.7 12
# stacked_plot 980 1.8 "10kΩ" 1.17 16

