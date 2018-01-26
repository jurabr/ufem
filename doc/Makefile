all: fem.pdf feminput.pdf tgfem.pdf femcmd.pdf mviz.pdf ufem.pdf ufem-cz.pdf

LATEX=latex
CSLATEX=cslatex

progdoc:
	(cd .. && doxygen fem.dox)
#	(cd .. && doxygen tui.dox)

ufem.pdf: ufem.dvi
	dvipdf ufem.dvi

ufem.dvi: ufem.tex
	latex ufem.tex
	latex ufem.tex

ufem.tex: ufem-doc.otl
	otl2a -t=latex <ufem-doc.otl | sed "s/_/\\\_/g" |tee $(@)

fem.dvi: ../fem/fem.1
	groff -man -Tdvi ../fem/fem.1 |tee $(@) >/dev/null

fem.pdf: fem.dvi
	dvipdf fem.dvi

feminput.dvi: ../fem/feminput.5
	groff -man -Tdvi ../fem/feminput.5 |tee $(@) >/dev/null

feminput.pdf: feminput.dvi
	dvipdf feminput.dvi

mviz.dvi: ../fem/mviz.1
	groff -man -Tdvi ../fem/mviz.1 |tee $(@) >/dev/null

mviz.pdf: mviz.dvi
	dvipdf mviz.dvi

tgfem.dvi: ../tgfem.1
	groff -man -Tdvi ../tgfem.1 |tee $(@) >/dev/null

tgfem.pdf: tgfem.dvi
	dvipdf tgfem.dvi

femcmd.dvi: ../femcmd.1
	groff -man -Tdvi ../femcmd.1 |tee $(@) >/dev/null

femcmd.pdf: femcmd.dvi
	dvipdf femcmd.dvi

# Czech guide:
ufem-cz.pdf: ufem-cz.ps
	ps2pdf ufem-cz.ps

ufem-cz.ps: ufem-cz.dvi
	dvips -t a4 ufem-cz.dvi -o

ufem-cz.dvi: ufem-cz.tex u-cz-uvod.tex u-lit-cz.tex u-cz-1sh.tex u-cz-gfx.tex u-cz-geo.tex u-cz-cmd.tex u-*cz-kp.tex u-cz-sol.tex u-cz-res.tex u-cz-inst.tex
	$(CSLATEX) ufem-cz.tex
	$(CSLATEX) ufem-cz.tex




clean:
	-rm -rf ufem.pdf fem.pdf feminput.pdf tgfem.pdf femcmd.pdf mviz.pdf *.dvi *.log *.aux core prog/tui/* prog/fem/* #ufem.tex 
