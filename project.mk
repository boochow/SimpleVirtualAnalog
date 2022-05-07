# #############################################################################
# Project Customization
# #############################################################################

PROJECT = simple_va

UCSRC =

UCXXSRC = simple_va.cpp

UINCDIR =

UDEFS =

ULIB = 

ULIBDIR =

WAVEGEN = gen_sawtooth_wavetable.py

default: sawtooth_wavetable.h all

sawtooth_wavetable.h: $(WAVEGEN)
	python $< >$@
