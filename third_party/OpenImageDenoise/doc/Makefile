SHELL := /bin/bash

webpages := $(addprefix www/, $(addsuffix .html, index news documentation gallery downloads related_projects legal))
process_version := $(addprefix tmp/, $(addsuffix .md, downloads readme_head))
tmptexfiles := $(addprefix tmp/, $(addsuffix .tex, overview changelog compilation api examples))
images_jpg := $(addprefix images/, $(addsuffix .jpg, moana_16spp_input moana_16spp_oidn moana_palms_16spp_input moana_palms_16spp_oidn bistro_64spp_input bistro_64spp_oidn bistro2_64spp_input bistro2_64spp_oidn sponza_16spp_input sponza_16spp_oidn mazda_64spp_input mazda_64spp_oidn mazda_512spp_color mazda_512spp_oidn mazda_512spp_albedo_firsthit mazda_512spp_albedo_nondeltahit mazda_512spp_normal_firsthit mazda_512spp_normal_nondeltahit villa_64spp_input villa_64spp_oidn villa2_64spp_input villa2_64spp_oidn artdeco_2048spp_input artdeco_2048spp_oidn))
images_png := $(addprefix images/, $(addsuffix .png, blacktocat))
images_fig :=
images_svg :=

images_fig2pdf := $(addprefix tmp/, $(addsuffix .pdf, $(images_fig)))
images_fig2png := $(addprefix images/, $(addsuffix .png, $(images_fig)))
webimages := $(addprefix www/, $(images_jpg) $(images_png) $(images_fig2png) $(addprefix images/, $(addsuffix .svg, $(images_svg))))
pdfimages := $(images_jpg) $(images_png) $(images_fig2pdf)

PANDOC := pandoc
PDMINVERSION := 2000000
PDOK := $(shell expr `$(PANDOC) --version|head -n 1|cut -d' ' -f 2| sed -e 's/\.\([0-9][0-9]\)/\1/g' -e 's/\.\([0-9]\)/0\1/g' -e 's/^[0-9]\{3,4\}$$/&00/' -e 's/^[0-9]\{5,6\}$$/&00/'` \>= $(PDMINVERSION))
ifneq "$(PDOK)" "1"
  $(error You need at least pandoc v2.0)
endif

all: www doc pdf
www: $(webpages) www/stylesheet.css $(webimages)
doc: ../README.md
pdf: ../readme.pdf

.PHONY: all www doc pdf clean realclean

if_is_devel_then = if [[ "${CI_BUILD_REF_NAME}" == "devel" || (-d ../.git && "`git rev-parse --abbrev-ref HEAD`" == "devel") ]] ; then

tmp/version: ../CMakeLists.txt $(wildcard ../.git/HEAD)
	sed -n $< -e "s/^SET(OIDN_VERSION_MAJOR \([0-9]\+\))/\1./ip" | tr -d '\n' > $@
	sed -n $< -e "s/^SET(OIDN_VERSION_MINOR \([0-9]\+\))/\1./ip" | tr -d '\n' >> $@
	sed -n $< -e "s/^SET(OIDN_VERSION_PATCH \([0-9]\+\))/\1/ip" | tr -d '\n' >> $@
	#$(if_is_devel_then) echo " (devel)"; else echo; fi >> $@

replace_version = sed -e "s/<OIDN_VERSION>/`cat tmp/version`/g" $< > $@

tmp/%.md: %.md tmp/version
	$(replace_version)

tmp/links.md: links.md

tmp/images_web.md: images.md
	sed -e "s/\.fig/.png/" -e "s@: @: https://openimagedenoise.github.io/images/@" $< > $@

tmp/images_local_png.md: images.md
	sed -e "s@: @: images/@" -e "s/\.fig/.png/" $< > $@

tmp/images_local_pdf.md: images.md
	sed -e "s@: @: images/@" -e "s@images/\(.*\)\.fig@tmp/\1.pdf@" $< > $@

changelog.md:
	ln -s ../CHANGELOG.md $@

## ----------------------------------------------------------------------------
## Directories
## ----------------------------------------------------------------------------

$(webpages) www/stylesheet.css: | wwwdir
wwwdir:
	@mkdir -p www

$(webimages): | wwwimgdir
wwwimgdir:
	@mkdir -p www/images

$(process_version) tmp/version ../readme.pdf $(tmptexfiles) tmp/api.md tmp/api_html.md tmp/links.md tmp/images_web.md tmp/images_local_pdf.md tmp/images_local_png.md: | tmpdir
tmpdir:
	@mkdir -p tmp

## ----------------------------------------------------------------------------
## Webpages
## ----------------------------------------------------------------------------

markdown2web = $(PANDOC) --email-obfuscation=none -f markdown $(filter-out webtemplate.html,$+) --template webtemplate -V select_$(basename $(@F)) -o $@

$(webpages): tmp/links.md tmp/images_local_png.md webtemplate.html

www/%.html:
	$(markdown2web)

www/stylesheet.css: stylesheet.css
	cp $< $@

tmp/api_html.md: api.md tmp/links.md tmp/images_local_png.md
	$(PANDOC) $+ --indented-code-classes=cpp -t markdown-fenced_code_attributes -o $@

www/index.html: teaser.html overview.md changelog.md
www/news.html: news.md
www/gallery.html: gallery.md
www/downloads.html: tmp/downloads.md compilation.md
www/related_projects.html: related_projects.md
www/legal.html: legal.md
www/documentation.html: documentation.md tmp/api_html.md examples.md

## ----------------------------------------------------------------------------
## Images
## ----------------------------------------------------------------------------

www/images/%.png: %.fig
	fig2dev -L png -S 4 $+ $@

www/images/%.svg: %.svg
	cp $+ $@

tmp/%.pdf: %.fig
	fig2dev -L pdf $+ $@

www/images/%: images/%
	cp $< $@

# try to download images if not linked; images may not be up to date
images/%:
	if [[ -d oidn-doc/images ]] ; then \
	  ln -fs oidn-doc/images ;\
	else \
	  if [[ -x "`which wget 2> /dev/null`" ]] ; then \
	    mkdir -p images ;\
	    wget https://openimagedenoise.github.io/$@ -O $@ ;\
	  else \
	    curl https://openimagedenoise.github.io/$@ --create-dirs -Lo $@ ;\
	  fi \
	fi

## ----------------------------------------------------------------------------
## Markdown
## ----------------------------------------------------------------------------

# Pandoc doesn't convert the alt text of an image to a caption in markdown, so we need to do it manually
convert_image_alt_to_caption = perl -0pe 's/!\[((?:\[.*?\]|.)+?)\]\((.*?)\)/![]\(\2)\n\1/sg'

tmp/api.md: api.md tmp/links.md tmp/images_web.md
	$(PANDOC) $+ --indented-code-classes=cpp -t markdown-fenced_code_attributes -o $@

../README.md: tmp/readme_head.md overview.md compilation.md documentation.md tmp/api.md examples.md tmp/links.md tmp/images_web.md
	$(PANDOC) $+ -t gfm | $(convert_image_alt_to_caption) > $@

## ----------------------------------------------------------------------------
## PDF
## ----------------------------------------------------------------------------

markdown2tex = $(PANDOC) --columns=72 --top-level-division=chapter --filter $+ -o $@

tmp/%.tex: filter-latex.py %.md tmp/links.md tmp/images_local_pdf.md
	$(markdown2tex)

tmp/api.tex: filter-latex.py api.md tmp/links.md tmp/images_local_pdf.md
	$(markdown2tex) --indented-code-classes=cpp

../readme.pdf: readme.tex $(tmptexfiles) $(wildcard oidn-doc/intel-spec.cls) preamble.tex tmp/version $(pdfimages)
	md5s=0;\
	auxf="tmp/`basename $< tex`aux";\
	newmd5s=`md5sum $$auxf 2> /dev/null`;\
	until [[ $$md5s == $$newmd5s ]]; do \
		md5s=$$newmd5s;\
		xelatex -output-directory=tmp $<;\
		newmd5s=`md5sum $$auxf`;\
	done
	cp tmp/$(basename $<).pdf $@

## ----------------------------------------------------------------------------
## Clean
## ----------------------------------------------------------------------------

clean:
	rm -rf www tmp changelog.md __pycache__

realclean: clean
	rm -irf images
