target = switches_and_lights
pcf_file = $(target).pcf
switches = switches.v
lights = lights.v
# clocks = clocks.v
other_verilog_modules = $(switches) $(lights) # $(clocks)

.PHONY: all clean

all: prog

touch:
	touch $(target).v

check:
	rm -fv $(target).json
	make $(target).json

prog: $(target).bin
	iceprog $(target).bin

$(target).bin: $(target).asc
	# icetime -d hx8k -c 12 -mtr $(target).rpt $(target).asc
	icepack $(target).asc $(target).bin

$(target).asc: $(target).json $(pcf_file)
	nextpnr-ice40 --hx8k --package ct256 --asc $(target).asc \
		--top $(target) \
		--pcf $(pcf_file) --json $(target).json --ignore-loops

$(target).json: $(target).v $(pcf_file) $(other_verilog_modules) Makefile
	yosys -ql $(target).log \
		-p "synth_ice40 -top $(target) -json $(target).json" \
		$(target).v
	# if (grep -i warning $(target).log > /dev/null) then false; fi

clean:
	rm -fv $(target).json $(target).log $(target).asc \
		$(target).rpt $(target).bin

notes:
	(cd ~/thesis/github/notes.new && make vi)

quotes:
	(cd ~/thesis/github/notes.new && make quotes)

ro:
	(cd ~/thesis/github/notes.new && make ro)

bib:
	(cd ~/thesis/github/bibtex && make vi)

pcf:
	vi $(pcf_file)

vi:
	vi $(target).v

switch:
	vi $(switches)

light:
	vi $(lights)

# clock:
# 	vi $(clocks)

changes:
	make clean
	git status
