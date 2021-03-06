
include_directories(${CMAKE_SOURCE_DIR}/projects/DocumentationGenerator ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(genStubsAndWrappers_SRCS genStubsAndWrappers.C)

kde4_add_executable(genStubsAndWrappers ${genStubsAndWrappers_SRCS})

target_link_libraries(genStubsAndWrappers ${KDE4_KDECORE_LIBS})


########### next target ###############

set(rosehswrap_LIB_SRCS WrapROSE.C WrapSage3.C WrapCxxContainers.C)

kde4_add_library(rosehswrap SHARED ${rosehswrap_LIB_SRCS})

target_link_libraries(rosehswrap ${KDE4_KDECORE_LIBS} rose)

set_target_properties(rosehswrap PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS rosehswrap ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############




#original Makefile.am contents follow:

#include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs
#
#NONGENERATED_HSFILES = LICENSE ROSE.hs \
#  Data/DataMc.hs \
#  ROSE/CxxContainers.hs \
#  ROSE/Sage3.hs \
#  ROSE/Sage3/Misc.hs \
#  ROSE/Sage3/Misc.hs-boot \
#  ROSE/Sage3/SYB.hs
#
#TEST_HSFILES = tests/identityTranslator/Main.hs \
#               tests/simplify/Main.hs
#
#TEST_CFILES = tests/simplify/simplifyTest.C
#
#EXTRA_DIST = $(NONGENERATED_HSFILES) sage3.h.C WrapCxxContainers.h $(TEST_HSFILES) $(TEST_CFILES) disabled.html
#
#if USE_HASKELL
#
#INCLUDES = \
#     $(ROSE_INCLUDES) -I$(top_srcdir)/projects/DocumentationGenerator
#
#noinst_PROGRAMS = genStubsAndWrappers
#lib_LTLIBRARIES = librosehswrap.la
#
#SRC_NONGENERATED_HSFILES = $(addprefix $(srcdir)/,$(NONGENERATED_HSFILES))
#
#LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS) -ldl $(LIBLTDL) $(top_builddir)/projects/DocumentationGenerator/libROSEDoxygen.a
#
#genStubsAndWrappers_SOURCES = genStubsAndWrappers.C
#
#ROSE/Sage3/Classes.hs WrapSage3Classes.C rose.cabal: run-genStubsAndWrappers-stamp
#	touch -c $@
#
#headersWithoutLineDirective: $(wildcard $(top_builddir)/src/frontend/SageIII/*.h)
#	mkdir -p $@
#	for file in $(top_builddir)/src/frontend/SageIII/*.h ; do \
#		base=`basename $$file` ; \
#		grep -v '^#line' $$file > $@/$$base ; \
#	done
#	touch $@
#
#run-genStubsAndWrappers-stamp: genStubsAndWrappers sage3.h.C rose.cabal.in headersWithoutLineDirective
#	mkdir -p ROSE/Sage3/Methods
#	./genStubsAndWrappers -rose:collectAllCommentsAndDirectives -isystem headersWithoutLineDirective -IheadersWithoutLineDirective $(ROSE_INCLUDES) -I$(top_builddir) -IheadersWithoutLineDirective $(srcdir)/sage3.h.C
#	touch $@
#
#librosehswrap_la_SOURCES = WrapROSE.C WrapSage3.C WrapCxxContainers.C
#nodist_librosehswrap_la_SOURCES = WrapSage3Classes.C
#librosehswrap_la_LIBADD = $(top_builddir)/src/librose.la
#
#all-local: cabal-build-stamp
#
#install-exec-local: cabal-install-stamp
#
#uninstall-local: cabal-configure-stamp
#	-$(GHCBIN)/runghc Setup.hs unregister
#	rm -rf $(prefix)/lib/rose-$(PACKAGE_VERSION_NUMERIC) $(prefix)/share/doc/rose-$(PACKAGE_VERSION_NUMERIC)
#
#clean-local: cabal-clean
#	rm -rf headersWithoutLineDirective
#	rm -f identityTranslator simplify simplifyTest
#	rm -f rose_*
#	rm -rf WrapSage3Classes.C ROSE/Sage3/Classes.hs ROSE/Sage3/Methods ROSE/Sage3/Methods.hs
#	rm -f tests/*/*.o tests/*/*.hi
#	if test "x$(srcdir)" != "x." ; then rm -rf Data ROSE tests ; fi
#	rm -f sage3.h.ti
#
#cabal-clean:
#	if [ -e cabal-configure-stamp ] ; then $(GHCBIN)/runghc Setup.hs clean ; fi
#	if test "x$(srcdir)" != "x." ; then rm -f $(NONGENERATED_HSFILES) ; fi
#	rm -f run-genStubsAndWrappers-stamp rose.cabal cabal-configure-stamp cabal-build-stamp cabal-install-stamp
#	rm -rf checkHome
#
## The distclean-generic rule removes the autoconf generated files,
## including Setup.hs which is used by the cabal-clean rule.  This causes
## problems for parallel builds.  So we impose a precondition that
## distclean-generic must run after cabal-clean
#distclean-generic: cabal-clean
#	-test -z "$(CONFIG_CLEAN_FILES)" || rm -f $(CONFIG_CLEAN_FILES)
#
#cabal-configure-stamp: ROSE/Sage3/Classes.hs rose.cabal Setup.hs $(SRC_NONGENERATED_HSFILES) librosehswrap.la
#	if test "x$(srcdir)" != "x." ; then \
#		for file in $(NONGENERATED_HSFILES) ; do \
#			if test ! $$file -nt $(srcdir)/$$file ; then \
#				dir=`dirname $$file` ; \
#				mkdir -p $$dir ; \
#				cp $(srcdir)/$$file $$dir ; \
#			fi ; \
#		done ; \
#	fi
#	$(GHCBIN)/runghc Setup.hs configure $(HS_CONFIG_OPTS)
#	touch $@
#
#cabal-build-stamp: cabal-configure-stamp
#	$(GHCBIN)/runghc Setup.hs build
#	touch $@
#
#cabal-install-stamp: cabal-build-stamp
#	$(GHCBIN)/runghc Setup.hs install
#	touch $@
#
#docs: cabal-configure-stamp
#	$(GHCBIN)/runghc Setup.hs haddock --html-location="http://haskell.org/ghc/docs/latest/html/libraries/\$$pkg/"
#
#check-local: check-identityTranslator check-simplify
#
#checkHome: cabal-build-stamp
#	mkdir -p checkHome
#	env HOME=checkHome $(GHCBIN)/runghc Setup.hs install --inplace --user
#	touch checkHome
#
#identityTranslator: tests/identityTranslator/Main.hs checkHome
#	mkdir -p tests/$@
#	env HOME=checkHome $(GHCBIN)/ghc -package rose -o $@ -odir tests/$@ -hidir tests/$@ -i $<
#
#check-identityTranslator: identityTranslator
#	./$< -c $(top_srcdir)/exampleTranslators/documentedExamples/simpleTranslatorExamples/testCode.C
#
#simplify: tests/simplify/Main.hs checkHome
#	mkdir -p tests/$@
#	env HOME=checkHome $(GHCBIN)/ghc -package rose -o $@ -odir tests/$@ -hidir tests/$@ -i $<
#
#check-simplify: tests/simplify/simplifyTest.C simplify
#	./simplify -o simplifyTest $<
#	./simplifyTest
#
#else
#
#docs: disabled.html
#	@echo "Haskell support disabled, unable to build Haddock docs!"
#	mkdir -p dist/doc/html/rose
#	cp $< dist/doc/html/rose/index.html
#
#endif
