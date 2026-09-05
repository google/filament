<?xml version='1.0'?>


<xsl:stylesheet
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
        xmlns:xlink="http://www.w3.org/1999/xlink"
        version="1.0">


        <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/xhtml/docbook.xsl"/>

<!-- This file contains the "customization layer" for the Docbook system. When a Docbook rule
doesn't do exactly what we need, we override the rule in this file with a modified rule. -->

<!-- Inserts the file style-css.xsl. This is embeddd into the <head /> tag of each resulting html file -->
        <xsl:include href="styles-css.xsl" />
        <xsl:include href="copyright.inc.xsl" />
        <xsl:param name="funcsynopsis.style">ansi</xsl:param>
        <xsl:param name="citerefentry.link" select="'1'"></xsl:param>
        <xsl:output indent="yes"/>

<!-- Set this param to a placeholder for the base URL of the external specification document. Include
the beginning of the 'namedest' function as well. See the script pageNumberLookup.rb to see
how this placeholder gets replaced by the actual spec URL and target page number. This placeholder
can be any string, and only needs to match the same placeholder string in pageNumberLookup.rb. -->
        <xsl:param name="SpecBaseUrl">http://www.khronos.org/registry/cl/specs/opencl-1.x-latest.pdf#namedest=</xsl:param>

<!-- This generates a URL based on the contents of Refentry title. However if there is a value
specified for the href attribute in Citerefentry, then it will create the URL out if that content instead -->
        <xsl:template name="generate.citerefentry.link">
                <xsl:choose>
                        <xsl:when test="@href">
                                <xsl:value-of select="@href"/>
                                <xsl:text>.html</xsl:text>
                        </xsl:when>
                        <xsl:otherwise>
                                <xsl:value-of select="refentrytitle"/>
                                <xsl:text>.html</xsl:text>
                        </xsl:otherwise>
                </xsl:choose>
        </xsl:template>

<!-- The following template creates the link for the Specification section  -->
        <xsl:template match="olink">
        <xsl:text disable-output-escaping="yes">&lt;a href="</xsl:text>
        <xsl:value-of select="$SpecBaseUrl" />
        <xsl:value-of select="@uri" />
        <xsl:text disable-output-escaping="yes">" target="OpenCL Spec"&gt;</xsl:text>
        <xsl:value-of select="." />
        <xsl:text disable-output-escaping="yes">&lt;/a&gt;</xsl:text>
        </xsl:template>

<!-- The following enables use of ulink for regular URLs on the pages-->
<xsl:template match="ulink">
        <xsl:text disable-output-escaping="yes">&lt;a href="</xsl:text>
        <xsl:value-of select="@url" />
        <xsl:text disable-output-escaping="yes">"&gt;</xsl:text>
        <xsl:value-of select="." />
        <xsl:text disable-output-escaping="yes">&lt;/a&gt;</xsl:text>
</xsl:template>


<!-- The following is the main set of templates for generating the web pages -->
        <xsl:template match="*" mode="process.root">
                <xsl:variable name="doc" select="self::*"/>
                <xsl:call-template name="user.preroot"/>
                <xsl:call-template name="root.messages"/>

                <html xmlns="http://www.w3.org/1999/xhtml" xmlns:pref="http://www.w3.org/2002/Math/preference" pref:renderer="mathplayer-dl">

                        <head>
                                <xsl:call-template name="system.head.content">
                                        <xsl:with-param name="node" select="$doc"/>
                                </xsl:call-template>

                                <xsl:call-template name="head.content">
                                        <xsl:with-param name="node" select="$doc"/>
                                </xsl:call-template>

                                <xsl:call-template name="user.head.content">
                                        <xsl:with-param name="node" select="$doc"/>
                                </xsl:call-template>
                        </head>

                        <body>
                                <xsl:call-template name="body.attributes"/>
                                <xsl:call-template name="user.header.content">
                                        <xsl:with-param name="node" select="$doc"/>
                                </xsl:call-template>
                                <xsl:apply-templates select="."/>  <!-- This line performs the magic! -->
                                <xsl:call-template name="user.footer.content">
                                        <xsl:with-param name="node" select="$doc"/>
                                </xsl:call-template>

                        </body>
                </html>
        </xsl:template>


<xsl:template match="/">
                <xsl:processing-instruction name="xml-stylesheet">
                        <xsl:text>type="text/xsl" href="mathml.xsl"</xsl:text>
                </xsl:processing-instruction>
                <xsl:apply-imports/>
</xsl:template>


<xsl:template match="funcdef/replaceable">
 <xsl:call-template name="inline.italicseq"/>
</xsl:template>


<!-- This inserts the style-css.xsl file into the HTML file -->
<xsl:template name="system.head.content">
  <xsl:param name="node" select="."/>
    <style type="text/css">
      <xsl:value-of select="$annotation.css"/>
    </style>
</xsl:template>


<!-- The templates gentext-refname and refnamediv are inserted here so
we can have the refname displayed as the H1 header on the page  -->
<xsl:template name="gentext-refname">
  <xsl:param name="key" select="local-name(.)"/>
  <xsl:param name="lang">
    <xsl:call-template name="l10n.language"/>
  </xsl:param>
  <xsl:value-of select="refname"/>
</xsl:template>

<xsl:template match="refnamediv">
  <div class="{name(.)}">
    <xsl:call-template name="anchor"/>
    <xsl:choose>
      <xsl:when test="preceding-sibling::refnamediv">
        <!-- no title on secondary refnamedivs! -->
      </xsl:when>
      <xsl:when test="$refentry.generate.name != 0">
        <h1>
      <xsl:call-template name="gentext-refname">
            <xsl:with-param name="key" select="'RefName'"/>
          </xsl:call-template>
        </h1>
      </xsl:when>
      <xsl:when test="$refentry.generate.title != 0">
        <h2>
          <xsl:choose>
            <xsl:when test="../refmeta/refentrytitle">
              <xsl:apply-templates select="../refmeta/refentrytitle"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:apply-templates select="refname[1]"/>
            </xsl:otherwise>
          </xsl:choose>
        </h2>
      </xsl:when>
    </xsl:choose>
    <p>
      <xsl:apply-templates/>
    </p>
  </div>
</xsl:template>


<!-- The templates refname and refpurpose are inserted here so that we can
modify the layout of these values on the HTML page -->
<xsl:template match="refname" />
<xsl:template match="refpurpose">
<xsl:apply-templates/>
</xsl:template>


<!-- The Link template allows us to embed links in the <funcprototype>,
even though this is not valid DocBook markup -->
<xsl:template match="link" mode="ansi-tabular">
  <xsl:apply-templates select="."/>
</xsl:template>


<!-- The following template enables the <replaceable> tag inside
    <funcdef>, <paramdef>, and <function> to generate <em> in the HTML output -->
<xsl:template match="funcdef/replaceable" mode="ansi-tabular">
 <xsl:call-template name="inline.italicseq"/>
</xsl:template>

<xsl:template match="paramdef/replaceable" mode="ansi-tabular">
 <xsl:call-template name="inline.italicseq"/>
</xsl:template>

<xsl:template match="function/replaceable" mode="ansi-nontabular">
 <xsl:call-template name="inline.italicseq"/>
</xsl:template>

<xsl:template match="refname/replaceable" mode="kr-nontabular">
 <xsl:call-template name="inline.italicseq"/>
</xsl:template>

<!-- The templates funcprototype and paramdef are here so we can modify the layout
of the synopsis so that it is not broken into so many columns in the table, control indenting, and more. -->

<!-- funcprototype: ansi, tabular -->

<xsl:template match="funcprototype" mode="ansi-tabular">
  <table border="0" summary="Function synopsis" cellspacing="0" cellpadding="0">
    <xsl:if test="following-sibling::funcprototype">
      <xsl:attribute name="style">padding-bottom: 1em</xsl:attribute>
    </xsl:if>
        <tr valign="bottom">
      <td>
        <xsl:apply-templates select="funcdef" mode="ansi-tabular"/>
                <xsl:apply-templates select="(void|varargs|paramdef)[1]" mode="ansi-tabular"/>
          </td>
    </tr>
    <xsl:for-each select="(void|varargs|paramdef)[position() &gt; 1]">
      <tr valign="top">
        <td>&#160;</td>
        <xsl:apply-templates select="." mode="ansi-tabular"/>
      </tr>
    </xsl:for-each>
  </table>
</xsl:template>


<xsl:template match="paramdef" mode="ansi-tabular">
  <xsl:choose>
    <xsl:when test="type and funcparams">
      <td>
        <xsl:apply-templates select="type" mode="kr-tabular-funcsynopsis-mode"/>
        <xsl:text>&#160;</xsl:text>
      </td>
      <td>
        <xsl:apply-templates select="type/following-sibling::node()" mode="kr-tabular-funcsynopsis-mode"/>
      </td>
    </xsl:when>
    <xsl:otherwise>
      <td>
        <xsl:apply-templates select="parameter/preceding-sibling::node()[not(self::parameter)]" mode="ansi-tabular"/>
        <xsl:text>&#160;</xsl:text>
<!--  </td>
      <td>-->
        <xsl:apply-templates select="parameter" mode="ansi-tabular"/>
        <xsl:apply-templates select="parameter/following-sibling::node()[not(self::parameter)]" mode="ansi-tabular"/>
        <xsl:choose>
          <xsl:when test="following-sibling::*">
            <xsl:text>, </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <code>)</code>
<!-- OpenCL functions do not end with a semi-colon.
           <xsl:text>;</xsl:text> -->
          </xsl:otherwise>
        </xsl:choose>
      </td>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>




<!-- The template refsect3 is inserted here to deal with the copyright. This
adds in the text from the include file copyright.inc.xsl -->
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
</xsl:stylesheet>
