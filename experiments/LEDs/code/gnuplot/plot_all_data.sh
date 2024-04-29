#!/bin/sh

raw_red_data="20230522.1701_405_nm_laser_I2C_red_LED.txt"
raw_blue_data="20230522.1750_405_nm_laser_I2C_blue_LED.txt"
raw_white_data="20230522.1855_405_nm_laser_I2C_white_LED.txt"
raw_green_data="20230522.1942_405_nm_laser_I2C_green_LED.txt"
raw_other_red_data="20230522.2039_405_nm_laser_I2C_other_red_LED.txt"
raw_deep_red_data="20230523.1820_405_nm_laser_I2C_deep_red_LED.txt"
raw_yellow_data="20230523.1928_405_nm_laser_I2C_yellow_LED.txt"
raw_oops_data="20230523.2029_405_nm_laser_I2C_yellow_LED_oops_I_left_it_on.txt"
raw_other_yellow_data="20230524.0720_405_nm_laser_I2C_other_yellow_LED.txt"
raw_pink_data="20230524.0825_405_nm_laser_I2C_pink_LED.txt"
raw_UV_data="20230524.0912_405_nm_laser_I2C_UV_led.txt"
raw_other_blue_data="20230524.0958_405_nm_laser_I2C_other_blue_LED.txt"
raw_Adafruit_red_data="20230524.1227_405_nm_laser_I2C_Adafruit_red_LED.txt"
raw_other_green_data="20230524.1314_405_nm_laser_I2C_other_green_Led.txt"
raw_last_red_data="20230524.1433_405_nm_laser_I2C_last_red_LED.txt"

raw_data_dir=raw_data
cooked_data_dir=cooked_data

# Parameters are: filename

function check_data_file_exists () {
  if [ ! -f ${raw_data_dir}/$1 ]; then
    echo "missing data file: $1"
    exit 1
  fi
}

# Make sure all the raw data exists.

for i in ${raw_red_data} ${raw_blue_data} ${raw_white_data} \
  ${raw_green_data} ${raw_other_red_data} ${raw_deep_red} \
  ${raw_yellow_data} ${raw_oops_data} ${raw_other_yellow_data} \
  ${raw_pink_data} ${raw_UV_data} ${raw_other_blue_data} \
  ${raw_Adafruit_red_data} ${raw_other_green_data} ${raw_last_red_data}
do
  check_data_file_exists ${i}
done

# Cook the raw data:

mkdir -p ${cooked_data_dir}

rm -f ${cooked_data_dir}/t*_cooked.txt
rm -rf ${cooked_data_dir}/*_contours.dat

# Parameters are: raw_data_filename, color

function cook_data () {
grep "^1," ${raw_data_dir}/$1 \
    | cut -d ',' -f 3-5 \
    | sed -e 's/,/ /g' \
    > ${cooked_data_dir}/cooked_$2_data.txt
}

cook_data ${raw_red_data} red
cook_data ${raw_blue_data} blue
cook_data ${raw_white_data} white
cook_data ${raw_green_data} green
cook_data ${raw_other_red_data} other_red
cook_data ${raw_deep_red_data} deep_red
cook_data ${raw_yellow_data} yellow
cook_data ${raw_oops_data} oops
cook_data ${raw_other_yellow_data} other_yellow
cook_data ${raw_pink_data} pink
cook_data ${raw_UV_data} UV
cook_data ${raw_other_blue_data} other_blue
cook_data ${raw_Adafruit_red_data} Adafruit_red
cook_data ${raw_other_green_data} other_green
cook_data ${raw_last_red_data} last_red

variables_file=variables.txt

# Parameters are: tag, LED_color, laser_wavelength, voltage, resistance,
#   logic_threshold

function single_plot () {
  rm -f ${variables_file}

  echo "tag = \"$1\"" >> ${variables_file}
  echo "LED_color = \"$2\"" >> ${variables_file}

  echo "laser_wavelength =" $3 "# nanometres" >> ${variables_file}
  echo "voltage =" $4 "# volts" >> ${variables_file}
  echo "resistance = \""$5 "pull-up\" # pull-up value in ohms" \
    >> ${variables_file}
  echo "logic_threshold =" $6 "# volts" >> ${variables_file}

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

# Parameters are: wavelength

function multiplot () {
  rm -f ${variables_file}

  echo "device_type = \"1N34A (Schottky silicon equivalent) DO-35 glass\"" \
    >> ${variables_file}

  echo "wavelength =" $1 "# nanometres" >> ${variables_file}

  gnuplot -p multiplot.gnuplot
}

# Use these for an overview:

# multiplot 780 parallel
# multiplot 780 perpendicular
# multiplot 780 diagonal

# multiplot 405

# Use these to look at individual plots in detail:

# single_plot red          "Red"          405 3.3 "2.2 kΩ" 2.0 1
single_plot   blue         "Blue"         405 3.3 "2.2 kΩ" 2.0 1
single_plot   white        "White"        405 3.3 "2.2 kΩ" 2.0 1
single_plot   green        "Green"        405 3.3 "2.2 kΩ" 2.0 1
# single_plot other_red    "Other red"    405 3.3 "2.2 kΩ" 2.0 1
# single_plot deep_red     "Deep red"     405 3.3 "2.2 kΩ" 2.0 1
# single_plot yellow       "Yellow"       405 3.3 "2.2 kΩ" 2.0 1
# single_plot oops         "oops"         405 3.3 "2.2 kΩ" 2.0 1
# single_plot other_yellow "Other yellow" 405 3.3 "2.2 kΩ" 2.0 1
single_plot   pink         "Pink"         405 3.3 "2.2 kΩ" 2.0 1
# single_plot UV           "UV"           405 3.3 "2.2 kΩ" 2.0 1
single_plot   other_blue   "Other blue"   405 3.3 "2.2 kΩ" 2.0 1
# single_plot Adafruit_red "Adafruit red" 405 3.3 "2.2 kΩ" 2.0 1
single_plot   other_green  "Other green"  405 3.3 "2.2 kΩ" 2.0 1
# single_plot last_red     "Last red"     405 3.3 "2.2 kΩ" 2.0 1

# Use these to look at the effect of elliptical beam axis orientation

# stacked_plot 780 5 "10kΩ" 2.0 4
# stacked_plot 780 3.3 "10kΩ" 2.0 8
# stacked_plot 780 2.5 "10kΩ" 1.7 12
# stacked_plot 780 1.8 "10kΩ" 1.17 16

