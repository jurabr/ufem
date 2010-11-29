# Global Makefile for "fem" software
# (C) 2003 Jiri Brozovsky
#
#

# TARGETS:

doc: doc/design.dvi
	(cd ../doc   ; $(MAKE) all)

fem: fem/fem
	(cd fem    ; $(MAKE) all)

tui: tui/fem_tui
	(cd tui    ; $(MAKE) all)


# CLEANING:

# clean everything:
clean:
	(cd fem    ; $(MAKE) clean)
	(cd common ; $(MAKE) clean)
	(cd cint   ; $(MAKE) clean)
	(cd meval  ; $(MAKE) clean)
	(cd fdb    ; $(MAKE) clean)
	(cd tui    ; $(MAKE) clean)
	(cd tests  ; $(MAKE) clean)
	#(cd doc   ; $(MAKE) clean)

# clean "fem" only:
clean_fem:
	(cd fem    ; $(MAKE) clean)

# clean "tui" only:
clean_tui:
	(cd tui    ; $(MAKE) clean)

 clean documentation only:
clean_doc:
	(cd ../doc   ; $(MAKE) clean)
