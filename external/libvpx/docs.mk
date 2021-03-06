INSTALL_MAPS += docs/%    docs/%
INSTALL_MAPS += src/%     %
INSTALL_MAPS += %         %

# Static documentation authored in doxygen
CODEC_DOX :=    mainpage.dox \
		keywords.dox \
		usage.dox \
		usage_cx.dox \
		usage_dx.dox \

# Other doxy files sourced in Markdown
TXT_DOX-$(CONFIG_VP8)          += vp8_api1_migration.dox
vp8_api1_migration.dox.DESC     = VP8 API 1.x Migration

TXT_DOX = $(call enabled,TXT_DOX)

%.dox: %.txt
	@echo "    [DOXY] $@"
	@$(SRC_PATH_BARE)/examples/gen_example_doxy.php \
             $(@:.dox=)  "$($@.DESC)" > $@ < $<


EXAMPLE_PATH += $(SRC_PATH_BARE) #for CHANGELOG, README, etc

doxyfile: libs.doxy_template libs.doxy examples.doxy
	@echo "    [CREATE] $@"
	@cat $^ > $@
	@echo "STRIP_FROM_PATH += $(SRC_PATH_BARE) $(BUILD_ROOT)" >> $@
	@echo "INPUT += $(addprefix $(SRC_PATH_BARE)/,$(CODEC_DOX))" >> $@;
	@echo "INPUT += $(TXT_DOX)" >> $@;
	@echo "EXAMPLE_PATH += $(EXAMPLE_PATH)" >> $@

CLEAN-OBJS += doxyfile $(wildcard docs/html/*)
docs/html/index.html: doxyfile $(CODEC_DOX) $(TXT_DOX)
	@echo "    [DOXYGEN] $<"
	@doxygen $<
DOCS-yes += docs/html/index.html

DIST-DOCS-yes = $(wildcard docs/html/*)
DIST-DOCS-$(CONFIG_CODEC_SRCS) += $(addprefix src/,$(CODEC_DOX))
DIST-DOCS-$(CONFIG_CODEC_SRCS) += src/libs.doxy_template
DIST-DOCS-yes                  += CHANGELOG
DIST-DOCS-yes                  += README
