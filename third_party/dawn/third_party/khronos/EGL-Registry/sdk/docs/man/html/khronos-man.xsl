<?xml version='1.0'?>
<xsl:stylesheet
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    version="1.0">

    <xsl:import href="http://docbook.sourceforge.net/release/xsl-ns/current/xhtml5/onechunk.xsl"/>

    <!-- These two options generate a file named ID.xhtml, where ID is the value
         of the document xml:id attribute, when processed with the chunking
         spreadsheet onechunk.xsl. See
         http://www.sagehill.net/docbookxsl/OneChunk.html -->
    <xsl:param name="use.id.as.filename">1</xsl:param>
    <xsl:param name="root.filename"></xsl:param>

    <!-- html.stylesheet adds the specified stylesheet to the page headers.
         docbook.css.link removes the default docbook.css.
    -->
    <xsl:param name="html.stylesheet">khronos-man.css</xsl:param>
    <xsl:param name="docbook.css.link">0</xsl:param>
    <xsl:param name="docbook.css.source"></xsl:param>

    <!-- Indent HTML, which requires using onechunk.xsl instead of docbook.xsl -->
    <xsl:param name="chunker.output.indent">yes</xsl:param>

    <!-- Style parameters -->
    <xsl:param name="funcsynopsis.style">ansi</xsl:param>
    <xsl:param name="citerefentry.link" select="'1'"></xsl:param>

    <!-- Generate links in href= attributes for <citerefentry>. Note
         that in the XSL-NS stylesheets, using the Docbook namespace
         prefix on the select expressions is *required*. Declaring
         xmlns:db at xsl:stylesheet scope causes other problems.

         If there is a value specified for the href attribute in
         citerefentry, then create the link from that attribute instead
         of the refentrytitle.
     -->
    <!-- The @href syntax isn't used by EGL at present -->
    <xsl:template xmlns:db="http://docbook.org/ns/docbook"
        name="generate.citerefentry.link">
        <xsl:choose>
            <xsl:when test="@href">
                <xsl:value-of select="@href"/>
                <xsl:text>.xhtml</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="db:refentrytitle"/>
                <xsl:text>.xhtml</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- Reasonable defaults for tables -->
    <xsl:param name="default.table.frame">all</xsl:param>
    <xsl:param name="table.borders.with.css" select="1"></xsl:param>
    <xsl:param name="table.cell.border.thickness">2px</xsl:param>
    <xsl:param name="table.frame.border.thickness">2px</xsl:param>

    <!-- Add MathJax <script> tags  to document <head> -->
    <!-- Now that the xmlns:db is declared above, it gets emitted on the
         <script> elements for unknown reasons
     -->
    <!-- Per http://docs.mathjax.org/en/latest/start.html#secure-access-to-the-cdn
         use their secure URI, instead of the HTTP URI
            src="http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
     -->
    <!-- This isn't used by EGL at present -->
<!--
    <xsl:template name="user.head.content">
        <script type="text/x-mathjax-config">
            MathJax.Hub.Config({
                MathML: {
                    extensions: ["content-mathml.js"]
                },
                tex2jax: {
                    inlineMath: [['$','$'], ['\\(','\\)']]
                }
            });
        </script>
        <script type="text/javascript"
            src="https://c328740.ssl.cf1.rackcdn.com/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML">
        </script>
    </xsl:template>
-->

    <!-- Add boilerplate to XHTML page title element describing which
         set of man pages this is. This should really be an XSL
         parameter which could be set on the command line -->
    <xsl:template name="user.head.title">
        <xsl:param name="node" select="."/>
        <xsl:param name="title"/>
        <title>
            <xsl:copy-of select="$title"/>
            <xsl:text> - EGL Reference Pages</xsl:text>
        </title>
    </xsl:template>

    <!-- Root template for processing the document -->
    <xsl:template match="*" mode="process.root">
        <xsl:variable name="doc" select="self::*"/>
        <xsl:call-template name="user.preroot"/>
        <xsl:call-template name="root.messages"/>
        <xsl:apply-templates select="."/>
    </xsl:template>

    <!-- The directives in egl-man.xsl used to be here -->

    <!-- The template refsect3 is inserted here to deal with the
         copyright. This adds in the text from the include file
         copyright.inc.xsl -->
    <!--
    <xsl:template match="refsect3">
        <div class="{name(.)}">
            <xsl:call-template name="language.attribute"/>
            <xsl:call-template name="anchor">
                <xsl:with-param name="conditional" select="0"/>
            </xsl:call-template>
            <xsl:apply-templates/>
            <xsl:value-of select="$copyright"/>
        </div>
    </xsl:template>
    -->

</xsl:stylesheet>
