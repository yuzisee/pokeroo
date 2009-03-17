#===================
#   Configuration
#===================

DOC_FOLDER := document
DOCBODY_FOLDER := document/body
DOCUMENT_NAME := ece499rpt
REFERENCES := $(DOC_FOLDER)/references.bib

#===============
#   Filenames
#===============

OUTPUT_PDF = $(DOCUMENT_NAME).pdf

MAIN_DOC = $(DOCUMENT_NAME)
MAIN_TEX = $(MAIN_DOC).tex
MAIN_BIB = $(DOCUMENT_NAME).bbl
MAIN_AUX = $(DOCUMENT_NAME).aux
MAIN_LOG = $(DOCUMENT_NAME).log
BIBTEX_LOG = $(DOCUMENT_NAME).blg
MAIN_TOC = $(DOCUMENT_NAME).toc
MAIN_LOT = $(DOCUMENT_NAME).lot
MAIN_LOF = $(DOCUMENT_NAME).lof


#==================
#   Other macros
#==================

SRC_TEX = $(wildcard $(DOCBODY_FOLDER)/*.tex) $(wildcard $(DOC_FOLDER)/*.tex) $(MAIN_TEX)

MAIN_AUX_OLD = $(MAIN_AUX).old
#Contents, tables, and figures
MAIN_TOC_OLD = $(MAIN_TOC).old
MAIN_LOT_OLD = $(MAIN_LOT).old
MAIN_LOF_OLD = $(MAIN_LOF).old

RERUN_BIBTEX_TOUCH = $(MAIN_BIB).run
RERUN_LATEX_REGEX := "^LaTeX.*Rerun to get .*right"
BIBTEX_ERROR_REGEX := "^(There was.*error"



#===================
#   Rules/Targets
#===================
all: $(OUTPUT_PDF)

clean:
	rm -vf $(MAIN_AUX_OLD) $(MAIN_AUX)
	rm -vf $(MAIN_TOC_OLD) $(MAIN_TOC)
	rm -vf $(MAIN_LOT_OLD) $(MAIN_LOT)
	rm -vf $(MAIN_LOF_OLD) $(MAIN_LOF)
	rm -vf $(MAIN_BIB)
	rm -vf $(MAIN_LOG) $(BIBTEX_LOG)
	rm -vf $(RERUN_BIBTEX_TOUCH)
	rm -vf $(OUTPUT_PDF)


$(MAIN_BIB): $(REFERENCES) $(RERUN_BIBTEX_TOUCH)
	( grep "bibdata" $(MAIN_AUX) && bibtex $(MAIN_DOC) ) || (touch $@ && echo Bibtex skipped... > $(BIBTEX_LOG))
	( grep "I found no .citation commands---while reading file $(MAIN_AUX)" $(BIBTEX_LOG) || ( grep -e $(BIBTEX_ERROR_REGEX) $(BIBTEX_LOG) && rm $@ $(RERUN_BIBTEX_TOUCH) && exit 1 ) ) || ls $@


$(OUTPUT_PDF): $(MAIN_BIB) $(SRC_TEX)
	cp $(MAIN_AUX) $(MAIN_AUX_OLD)
	cp $(MAIN_TOC) $(MAIN_TOC_OLD)
	cp $(MAIN_LOT) $(MAIN_LOT_OLD)
	cp $(MAIN_LOF) $(MAIN_LOF_OLD)
	pdflatex -halt-on-error $(MAIN_DOC)
#Test for rerun.
#Note that "rm -f" is required because the pdf might be open in a viewer somewhere
	@( grep -e $(RERUN_LATEX_REGEX) $(MAIN_LOG) && rm -f $@ ) || echo No change in charts/cross-references
	@(diff -q $(MAIN_AUX) $(MAIN_AUX_OLD) && echo No change in $(MAIN_AUX)) || touch $(RERUN_BIBTEX_TOUCH)
	@(diff -q $(MAIN_TOC) $(MAIN_TOC_OLD) && echo No change in Table of Contents) || rm -f $@
	@(diff -q $(MAIN_LOT) $(MAIN_LOT_OLD) && echo No change in List of Tables) || rm -f $@
	@(diff -q $(MAIN_LOF) $(MAIN_LOF_OLD) && echo No change in List of Figures) || rm -f $@
#	(test -e $(OUTPUT_PDF) && echo Done) ||
	make $@

	
$(RERUN_BIBTEX_TOUCH): 
#If the token file hasn't been created yet, we also probably need MAIN_AUX
	pdflatex $(MAIN_DOC)
	touch $@
	
