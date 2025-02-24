<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:exsl="http://exslt.org/common"
		xmlns:db = "http://docbook.org/ns/docbook"
		xmlns:xlink="http://www.w3.org/1999/xlink"
                exclude-result-prefixes="exsl db"
                version="1.0">

<!--
# ======================================================================
# This file is part of DocBook V5.0CR5
#
# Copyright 2005 Norman Walsh, Sun Microsystems, Inc., and the
# Organization for the Advancement of Structured Information
# Standards (OASIS).
#
# Release: $Id: db4-upgrade.xsl 9828 2013-11-03 21:45:22Z tom_schr $
#
# Permission to use, copy, modify and distribute this stylesheet
# and its accompanying documentation for any purpose and without fee
# is hereby granted in perpetuity, provided that the above copyright
# notice and this paragraph appear in all copies. The copyright
# holders make no representation about the suitability of the schema
# for any purpose. It is provided "as is" without expressed or implied
# warranty.
#
# Please direct all questions, bug reports, or suggestions for changes
# to the docbook@lists.oasis-open.org mailing list. For more
# information, see http://www.oasis-open.org/docbook/.
#
# ======================================================================
-->

<xsl:param name="db5.version" select="'5.0'"/> <!-- DocBook version for the output 5.0 and 5.1 only current values -->
<xsl:param name="db5.version.string" select="$db5.version"/> <!-- Set this if you want a local version number -->
<xsl:param name="keep.numbered.sections" select="'0'"/> <!-- Set to 1 to keep numbered sections, default changes to recursive -->

<xsl:variable name="version" select="'1.1'"/> <!-- version of this transform -->

<xsl:output method="xml" encoding="utf-8" indent="no" omit-xml-declaration="yes"/>

<xsl:preserve-space elements="*"/>
<xsl:param name="rootid">
  <xsl:choose>
  <xsl:when test="/*/@id">
    <xsl:value-of select="/*/@id"/>
  </xsl:when>
  <xsl:otherwise>
    <xsl:text>UNKNOWN</xsl:text>
  </xsl:otherwise>
  </xsl:choose>
</xsl:param>

<xsl:param name="defaultDate" select="''"/>

<xsl:template match="/">
  <xsl:variable name="converted">
    <xsl:apply-templates/>
  </xsl:variable>
  <xsl:comment>
    <xsl:text> Converted by db4-upgrade version </xsl:text>
    <xsl:value-of select="$version"/>
    <xsl:text> </xsl:text>
  </xsl:comment>
  <xsl:text>&#10;</xsl:text>
  <xsl:apply-templates select="exsl:node-set($converted)/*" mode="addNS"/>
</xsl:template>

<!-- Convert numbered sections into recursive sections, unless
     $keep.numbered.sections is set to '1'  -->
<xsl:template match="sect1|sect2|sect3|sect4|sect5|section"
              priority="200">
  <xsl:choose>
    <xsl:when test="$keep.numbered.sections = '1'">
      <xsl:element name="{local-name(.)}">
        <xsl:call-template name="copy.attributes"/>
        <xsl:apply-templates/>
      </xsl:element>
    </xsl:when>
    <xsl:otherwise>
      <section>
        <xsl:call-template name="copy.attributes"/>
        <xsl:apply-templates/>
      </section>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!-- This is the template for the elements (book, article, set) that allow
     title, subtitle, and titleabbrev before (or in) info, but not after.
     If title, subtitle, or titleabbrev exist both inside and outside the
     info block, everything is moved inside. Otherwise things are left as is. -->
<xsl:template match="bookinfo|articleinfo|artheader|setinfo" priority="200">
  <xsl:variable name="title.inside.info">
    <xsl:choose>
      <xsl:when test="./title or ./subtitle or ./titleabbrev">
        <xsl:text>1</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="title.outside.info">
    <xsl:choose>
      <xsl:when test="preceding-sibling::title or preceding-sibling::subtitle or preceding-sibling::titleabbrev">
        <xsl:text>1</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <info>
    <xsl:if test="$title.inside.info = '1' and $title.outside.info = '1'">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Found title|subtitle|titleabbrev both inside and outside </xsl:text><xsl:value-of select="local-name(.)"/>
          <xsl:text>. Moving all inside info element.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:if test="preceding-sibling::title and not(./title)">
        <xsl:apply-templates select="preceding-sibling::title" mode="copy"/>
      </xsl:if>
      <xsl:if test="preceding-sibling::subtitle and not(./subtitle)">
        <xsl:apply-templates select="preceding-sibling::subtitle" mode="copy"/>
      </xsl:if>
      <xsl:if test="preceding-sibling::titleabbrev and not(./titleabbrev)">
        <xsl:apply-templates select="preceding-sibling::titleabbrev" mode="copy"/>
      </xsl:if>
    </xsl:if>
    <xsl:apply-templates/>
  </info>
</xsl:template>
<!-- This is the template for the elements (all except book, article, set) that
     allow title, subtitle, and titleabbrev after (or in) info, but not before.
     If an info element exists, and there is a title, subtitle, or titleabbrev
     after the info element, then the element is moved inside the info block.
     However, if a duplicate element exists inside the info element, that element
     is kept, and the one outside is dropped.-->
<xsl:template match="appendixinfo|blockinfo|bibliographyinfo|glossaryinfo
                     |indexinfo|setindexinfo|chapterinfo
                     |sect1info|sect2info|sect3info|sect4info|sect5info|sectioninfo
                     |refsect1info|refsect2info|refsect3info|refsectioninfo
                     |referenceinfo|partinfo
                     |objectinfo|prefaceinfo|refsynopsisdivinfo
                     |screeninfo|sidebarinfo"
              priority="200">
  <xsl:variable name="title.inside.info">
    <xsl:choose>
      <xsl:when test="./title or ./subtitle or ./titleabbrev">
        <xsl:text>1</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

    <!-- place title/subtitle/titleabbrev inside if any of them are already inside.
         otherwise place them before. -->
    <xsl:choose>
      <xsl:when test="$title.inside.info = '0'">
        <xsl:call-template name="emit-message">
          <xsl:with-param name="message">
            <xsl:text>Keeping one or more title elements before </xsl:text><xsl:value-of select="local-name(.)"/>
          </xsl:with-param>
        </xsl:call-template>

        <xsl:if test="following-sibling::title and not(./title)">
          <xsl:apply-templates select="following-sibling::title" mode="copy"/>
         </xsl:if>
        <xsl:if test="following-sibling::subtitle and not(./subtitle)">
          <xsl:apply-templates select="following-sibling::subtitle" mode="copy"/>
        </xsl:if>
        <xsl:if test="following-sibling::titleabbrev and not(./titleabbrev)">
          <xsl:apply-templates select="following-sibling::titleabbrev" mode="copy"/>
        </xsl:if>
         <info>
          <xsl:call-template name="copy.attributes"/>
          <xsl:apply-templates/>
        </info>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="emit-message">
          <xsl:with-param name="message">
            <xsl:text>Moving one or more title elements into </xsl:text><xsl:value-of select="local-name(.)"/>
          </xsl:with-param>
        </xsl:call-template>
        <info>
          <xsl:call-template name="copy.attributes"/>
          <xsl:if test="following-sibling::title and not(./title)">
            <xsl:apply-templates select="following-sibling::title" mode="copy"/>
          </xsl:if>
          <xsl:if test="following-sibling::subtitle and not(./subtitle)">
            <xsl:apply-templates select="following-sibling::subtitle" mode="copy"/>
          </xsl:if>
          <xsl:if test="following-sibling::titleabbrev and not(./titleabbrev)">
            <xsl:apply-templates select="following-sibling::titleabbrev" mode="copy"/>
          </xsl:if>
          <xsl:apply-templates/>
        </info>
      </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<xsl:template match="refentryinfo"
              priority="200">
  <info>
    <xsl:call-template name="copy.attributes"/>

    <xsl:if test="title">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Discarding title from refentryinfo!</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="titleabbrev">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Discarding titleabbrev from refentryinfo!</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>

    <xsl:if test="subtitle">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Discarding subtitle from refentryinfo!</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>

    <xsl:apply-templates/>
  </info>
</xsl:template>

<xsl:template match="refmiscinfo"
              priority="200">
  <refmiscinfo>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress" select="'class'"/>
    </xsl:call-template>
    <xsl:if test="@class">
      <xsl:choose>
	<xsl:when test="@class = 'source'
		        or @class = 'version'
		        or @class = 'manual'
		        or @class = 'sectdesc'
		        or @class = 'software'">
	  <xsl:attribute name="class">
	    <xsl:value-of select="@class"/>
	  </xsl:attribute>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:attribute name="class">
	    <xsl:value-of select="'other'"/>
	  </xsl:attribute>
	  <xsl:attribute name="otherclass">
	    <xsl:value-of select="@class"/>
	  </xsl:attribute>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:if>
    <xsl:apply-templates/>
  </refmiscinfo>
</xsl:template>

<xsl:template match="corpauthor" priority="200">
  <author>
    <xsl:call-template name="copy.attributes"/>
    <orgname>
      <xsl:apply-templates/>
    </orgname>
  </author>
</xsl:template>

<xsl:template match="corpname" priority="200">
  <orgname>
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </orgname>
</xsl:template>

<xsl:template match="author[not(personname)]|editor[not(personname)]|othercredit[not(personname)]" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes"/>
    <personname>
      <xsl:apply-templates select="honorific|firstname|surname|othername|lineage"/>
    </personname>
    <xsl:apply-templates select="*[not(self::honorific|self::firstname|self::surname
                                   |self::othername|self::lineage)]"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="address|programlisting|screen|funcsynopsisinfo
                     |classsynopsisinfo" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress" select="'format'"/>
    </xsl:call-template>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<!-- Suppress attributes with default values (i.e., added implicitly by DTD) -->
<xsl:template match="productname" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress.default" select="'class=trade'"/>
    </xsl:call-template>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<xsl:template match="orderedlist" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress.default" select="'inheritnum=ignore continuation=restarts'"/>
    </xsl:call-template>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<xsl:template match="literallayout" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress" select="'format'"/><!-- Dropped entirely in DB5 -->
      <xsl:with-param name="suppress.default" select="'class=normal'"/>
    </xsl:call-template>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<xsl:template match="equation" priority="200">
  <xsl:choose>
    <xsl:when test="not(title)">
      <xsl:call-template name="emit-message">
        <xsl:with-param
            name="message"
            >Convert equation without title to informal equation.</xsl:with-param>
      </xsl:call-template>
      <informalequation>
        <xsl:call-template name="copy.attributes"/>
        <xsl:apply-templates/>
      </informalequation>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy>
        <xsl:call-template name="copy.attributes"/>
        <xsl:apply-templates/>
      </xsl:copy>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="imagedata|videodata|audiodata|textdata" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress" select="'srccredit'"/>
    </xsl:call-template>
    <xsl:if test="@srccredit">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Check conversion of srccredit </xsl:text>
          <xsl:text>(othercredit="srccredit").</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
      <info>
        <othercredit class="other" otherclass="srccredit">
          <orgname>???</orgname>
          <contrib>
            <xsl:value-of select="@srccredit"/>
          </contrib>
        </othercredit>
      </info>
    </xsl:if>
  </xsl:copy>
</xsl:template>

<xsl:template match="sgmltag" priority="200">
  <tag>
    <xsl:call-template name="copy.attributes"/>
    <xsl:if test="@class = 'sgmlcomment'">
      <xsl:attribute name="class">comment</xsl:attribute>
    </xsl:if>
    <xsl:apply-templates/>
  </tag>
</xsl:template>

<xsl:template match="inlinegraphic[@format='linespecific']" priority="210">
  <textobject>
    <textdata>
      <xsl:call-template name="copy.attributes"/>
    </textdata>
  </textobject>
</xsl:template>

<xsl:template match="inlinegraphic" priority="200">
  <inlinemediaobject>
    <imageobject>
      <imagedata>
	<xsl:call-template name="copy.attributes"/>
      </imagedata>
    </imageobject>
  </inlinemediaobject>
</xsl:template>

<xsl:template match="graphic[@format='linespecific']" priority="210">
  <mediaobject>
    <textobject>
      <textdata>
	<xsl:call-template name="copy.attributes"/>
      </textdata>
    </textobject>
  </mediaobject>
</xsl:template>

<xsl:template match="graphic" priority="200">
  <mediaobject>
    <imageobject>
      <imagedata>
	<xsl:call-template name="copy.attributes"/>
      </imagedata>
    </imageobject>
  </mediaobject>
</xsl:template>

<xsl:template match="pubsnumber" priority="200">
  <biblioid class="pubsnumber">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </biblioid>
</xsl:template>

<xsl:template match="invpartnumber" priority="200">
  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>Converting invpartnumber to biblioid otherclass="invpartnumber".</xsl:text>
    </xsl:with-param>
  </xsl:call-template>
  <biblioid class="other" otherclass="invpartnumber">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </biblioid>
</xsl:template>

<xsl:template match="contractsponsor" priority="200">
  <xsl:variable name="contractnum"
                select="preceding-sibling::contractnum|following-sibling::contractnum"/>

  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>Converting contractsponsor to othercredit="contractsponsor".</xsl:text>
    </xsl:with-param>
  </xsl:call-template>

  <othercredit class="other" otherclass="contractsponsor">
    <orgname>
      <xsl:call-template name="copy.attributes"/>
      <xsl:apply-templates/>
    </orgname>
    <xsl:for-each select="$contractnum">
      <contrib role="contractnum">
        <xsl:apply-templates select="node()"/>
      </contrib>
    </xsl:for-each>
  </othercredit>
</xsl:template>

<xsl:template match="contractnum" priority="200">
  <xsl:if test="not(preceding-sibling::contractsponsor
                    |following-sibling::contractsponsor)
                and not(preceding-sibling::contractnum)">
    <xsl:call-template name="emit-message">
      <xsl:with-param name="message">
        <xsl:text>Converting contractnum to othercredit="contractnum".</xsl:text>
      </xsl:with-param>
    </xsl:call-template>

    <othercredit class="other" otherclass="contractnum">
      <orgname>???</orgname>
      <xsl:for-each select="self::contractnum
                            |preceding-sibling::contractnum
                            |following-sibling::contractnum">
        <contrib>
          <xsl:apply-templates select="node()"/>
        </contrib>
      </xsl:for-each>
    </othercredit>
  </xsl:if>
</xsl:template>

<xsl:template match="isbn|issn" priority="200">
  <biblioid class="{local-name(.)}">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </biblioid>
</xsl:template>

<xsl:template match="biblioid[count(*) = 1
		              and ulink
			      and normalize-space(text()) = '']" priority="200">
  <biblioid xlink:href="{ulink/@url}">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates select="ulink/node()"/>
  </biblioid>
</xsl:template>

<xsl:template match="authorblurb" priority="200">
  <personblurb>
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </personblurb>
</xsl:template>

<xsl:template match="collabname" priority="200">
  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>Check conversion of collabname </xsl:text>
      <xsl:text>(orgname role="collabname").</xsl:text>
    </xsl:with-param>
  </xsl:call-template>
  <orgname role="collabname">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </orgname>
</xsl:template>

<xsl:template match="modespec" priority="200">
  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>Discarding modespec (</xsl:text>
      <xsl:value-of select="."/>
      <xsl:text>).</xsl:text>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template match="mediaobjectco" priority="200">
  <mediaobject>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates/>
  </mediaobject>
</xsl:template>

<xsl:template match="remark">
  <!-- get rid of any embedded markup if the version is 5.0. If it's > 5.0, leave markup in. -->
  <remark>
    <xsl:copy-of select="@*"/>
    <xsl:choose>
      <xsl:when test="$db5.version>5.0">
        <xsl:apply-templates/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="."/>
      </xsl:otherwise>
    </xsl:choose>
  </remark>
</xsl:template>

<xsl:template match="biblioentry/title
                     |bibliomset/title
                     |biblioset/title
                     |bibliomixed/title" priority="400">
  <citetitle>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates/>
  </citetitle>
</xsl:template>

<xsl:template match="biblioentry/titleabbrev|biblioentry/subtitle
                     |bibliomset/titleabbrev|bibliomset/subtitle
                     |biblioset/titleabbrev|biblioset/subtitle
                     |bibliomixed/titleabbrev|bibliomixed/subtitle"
	      priority="400">
  <xsl:copy>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<xsl:template match="biblioentry/contrib
                     |bibliomset/contrib
                     |bibliomixed/contrib" priority="200">
  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>Check conversion of contrib </xsl:text>
      <xsl:text>(othercontrib="contrib").</xsl:text>
    </xsl:with-param>
  </xsl:call-template>
  <othercredit class="other" otherclass="contrib">
    <orgname>???</orgname>
    <contrib>
      <xsl:call-template name="copy.attributes"/>
      <xsl:apply-templates/>
    </contrib>
  </othercredit>
</xsl:template>

<xsl:template match="link" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<xsl:template match="ulink" priority="200">
  <xsl:choose>
    <xsl:when test="node()">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Converting ulink to link.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>

      <link xlink:href="{@url}">
	<xsl:call-template name="copy.attributes">
	  <xsl:with-param name="suppress" select="'url'"/>
	</xsl:call-template>
	<xsl:apply-templates/>
      </link>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Converting ulink to uri.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>

      <uri xlink:href="{@url}">
	<xsl:call-template name="copy.attributes">
	  <xsl:with-param name="suppress" select="'url'"/>
	</xsl:call-template>
	<xsl:value-of select="@url"/>
      </uri>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="olink" priority="200">
  <xsl:if test="@linkmode">
    <xsl:call-template name="emit-message">
      <xsl:with-param name="message">
        <xsl:text>Discarding linkmode on olink.</xsl:text>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:if>

  <xsl:choose>
    <xsl:when test="@targetdocent">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Converting olink targetdocent to targetdoc.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>

      <olink targetdoc="{unparsed-entity-uri(@targetdocent)}">
	<xsl:for-each select="@*">
	  <xsl:if test="name(.) != 'targetdocent'
	                and name(.) != 'linkmode'">
	    <xsl:copy/>
	  </xsl:if>
	</xsl:for-each>
	<xsl:apply-templates/>
      </olink>
    </xsl:when>
    <xsl:otherwise>
      <olink>
	<xsl:for-each select="@*">
	  <xsl:if test="name(.) != 'linkmode'">
	    <xsl:copy/>
	  </xsl:if>
	</xsl:for-each>
	<xsl:apply-templates/>
      </olink>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="biblioentry/firstname
                     |biblioentry/surname
                     |biblioentry/othername
                     |biblioentry/lineage
                     |biblioentry/honorific
                     |bibliomset/firstname
                     |bibliomset/surname
                     |bibliomset/othername
                     |bibliomset/lineage
                     |bibliomset/honorific" priority="200">
  <xsl:choose>
    <xsl:when test="preceding-sibling::firstname
                    |preceding-sibling::surname
                    |preceding-sibling::othername
                    |preceding-sibling::lineage
                    |preceding-sibling::honorific">
      <!-- nop -->
    </xsl:when>
    <xsl:otherwise>
      <personname>
        <xsl:apply-templates select="../firstname
                                     |../surname
                                     |../othername
                                     |../lineage
                                     |../honorific" mode="copy"/>
      </personname>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="areaset" priority="200">
  <xsl:copy>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress" select="'coords'"/>
    </xsl:call-template>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<xsl:template match="date|pubdate" priority="200">
  <xsl:variable name="rp1" select="substring-before(normalize-space(.), ' ')"/>
  <xsl:variable name="rp2"
		select="substring-before(substring-after(normalize-space(.), ' '),
		                         ' ')"/>
  <xsl:variable name="rp3"
		select="substring-after(substring-after(normalize-space(.), ' '), ' ')"/>

  <xsl:variable name="p1">
    <xsl:choose>
      <xsl:when test="contains($rp1, ',')">
	<xsl:value-of select="substring-before($rp1, ',')"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="$rp1"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="p2">
    <xsl:choose>
      <xsl:when test="contains($rp2, ',')">
	<xsl:value-of select="substring-before($rp2, ',')"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="$rp2"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="p3">
    <xsl:choose>
      <xsl:when test="contains($rp3, ',')">
	<xsl:value-of select="substring-before($rp3, ',')"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="$rp3"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="date">
    <xsl:choose>
      <xsl:when test="string($p1+1) != 'NaN' and string($p3+1) != 'NaN'">
	<xsl:choose>
	  <xsl:when test="$p2 = 'Jan' or $p2 = 'January'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-01-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Feb' or $p2 = 'February'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-02-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Mar' or $p2 = 'March'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-03-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Apr' or $p2 = 'April'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-04-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'May'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-05-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Jun' or $p2 = 'June'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-06-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Jul' or $p2 = 'July'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-07-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Aug' or $p2 = 'August'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-08-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Sep' or $p2 = 'September'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-09-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Oct' or $p2 = 'October'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-10-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Nov' or $p2 = 'November'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-11-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p2 = 'Dec' or $p2 = 'December'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-12-</xsl:text>
	    <xsl:number value="$p1" format="01"/>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:apply-templates/>
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:when>
      <xsl:when test="string($p2+1) != 'NaN' and string($p3+1) != 'NaN'">
	<xsl:choose>
	  <xsl:when test="$p1 = 'Jan' or $p1 = 'January'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-01-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Feb' or $p1 = 'February'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-02-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Mar' or $p1 = 'March'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-03-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Apr' or $p1 = 'April'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-04-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'May'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-05-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Jun' or $p1 = 'June'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-06-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Jul' or $p1 = 'July'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-07-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Aug' or $p1 = 'August'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-08-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Sep' or $p1 = 'September'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-09-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Oct' or $p1 = 'October'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-10-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Nov' or $p1 = 'November'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-11-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:when test="$p1 = 'Dec' or $p1 = 'December'">
	    <xsl:number value="$p3" format="0001"/>
	    <xsl:text>-12-</xsl:text>
	    <xsl:number value="$p2" format="01"/>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:apply-templates/>
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:when>
      <xsl:otherwise>
	<xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:choose>
    <xsl:when test="normalize-space($date) != normalize-space(.)">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Converted </xsl:text>
          <xsl:value-of select="normalize-space(.)"/>
          <xsl:text> into </xsl:text>
          <xsl:value-of select="$date"/>
          <xsl:text> for </xsl:text>
          <xsl:value-of select="name(.)"/>
        </xsl:with-param>
      </xsl:call-template>

      <xsl:copy>
	<xsl:copy-of select="@*"/>
	<xsl:value-of select="$date"/>
      </xsl:copy>
    </xsl:when>

    <xsl:when test="$defaultDate != ''">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Unparseable date: </xsl:text>
          <xsl:value-of select="normalize-space(.)"/>
          <xsl:text> in </xsl:text>
          <xsl:value-of select="name(.)"/>
          <xsl:text> (Using default: </xsl:text>
          <xsl:value-of select="$defaultDate"/>
          <xsl:text>)</xsl:text>
        </xsl:with-param>
      </xsl:call-template>

      <xsl:copy>
	<xsl:copy-of select="@*"/>
	<xsl:copy-of select="$defaultDate"/>
	<xsl:comment>
	  <xsl:value-of select="."/>
	</xsl:comment>
      </xsl:copy>
    </xsl:when>

    <xsl:otherwise>
      <!-- these don't really matter anymore
           <xsl:call-template name="emit-message">
           <xsl:with-param name="message">
           <xsl:text>Unparseable date: </xsl:text>
           <xsl:value-of select="normalize-space(.)"/>
           <xsl:text> in </xsl:text>
           <xsl:value-of select="name(.)"/>
           </xsl:with-param>
           </xsl:call-template>
      -->
      <xsl:copy>
	<xsl:copy-of select="@*"/>
	<xsl:apply-templates/>
      </xsl:copy>
    </xsl:otherwise>
  </xsl:choose>      
</xsl:template>

<xsl:template match="title|subtitle|titleabbrev" priority="300">
  <xsl:variable name="local.name" select="local-name(.)"/>
  <xsl:variable name="parent.name" select="local-name(..)"/>

  <!-- First three tests drop element if parent ZZZ already has
       ZZZinfo/title (or subtitle, or titleabbrev). -->
  <xsl:choose>
    <xsl:when test="../*[local-name(.) = concat($parent.name, 'info')]/*[local-name(.) = $local.name]">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Check </xsl:text>
          <xsl:value-of select="$parent.name"/>
          <xsl:text> title.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <!-- Before 4.0, <articleinfo> was known as <artheader> -->
    <xsl:when test="$parent.name = 'article' and ../artheader/*[local-name(.) = $local.name]">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Check </xsl:text>
          <xsl:value-of select="$parent.name"/>
          <xsl:text> title.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
   <xsl:when test="../blockinfo/*[local-name(.) = $local.name]">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Check </xsl:text>
          <xsl:value-of select="$parent.name"/>
          <xsl:text> title.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <!-- always drop title, subtitle, and titleabbrev from refentryinfo -->
    <xsl:when test="$parent.name = 'refentryinfo'">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Removing title in refentryinfo.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>      
    </xsl:when>
    <!-- Also drop title, subtitle, and titleabbrev when they appear after info.
         The title is picked up and moved either into or before the info element
         in the templates that handle info elements. -->
    <xsl:when test="preceding-sibling::*[local-name(.) = concat($parent.name, 'info')]">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Removing </xsl:text><xsl:value-of select="$local.name"/>
          <xsl:text> after </xsl:text><xsl:value-of select="$parent.name"/><xsl:text>info. Moved before or inside info.</xsl:text>
        </xsl:with-param>
      </xsl:call-template> 
    </xsl:when>
    <!-- this covers block elements that use blockinfo-->
    <xsl:when test="preceding-sibling::blockinfo">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Removing </xsl:text><xsl:value-of select="$local.name"/>
          <xsl:text> after blockinfo. Moved before or inside info.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <!-- The next clause removes title, subtitle, or titleabbrev if it was
         moved inside the info block. Only happens when one or more of these
         elements occurs both inside and outside the info element. -->
    <xsl:when test="following-sibling::bookinfo[title|subtitle|titleabbrev] or
                    following-sibling::articleinfo[title|subtitle|titleabbrev] or
                    following-sibling::artheader[title|subtitle|titleabbrev] or
                    following-sibling::setinfo[title|subtitle|titleabbrev]">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Removing </xsl:text><xsl:value-of select="$local.name"/>
          <xsl:text>. Has been moved inside info.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy>
        <xsl:call-template name="copy.attributes"/>
        <xsl:apply-templates/>
      </xsl:copy>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- Allow abstract inside valid biblio* elements, and inside info elements, otherwise drop -->
<xsl:template match="abstract" priority="300">
  <xsl:choose>
    <xsl:when test="not(contains(name(parent::*),'info'))
                    and not(parent::biblioentry) and not(parent::bibliomixed)
                    and not(parent::bibliomset)  and not(parent::biblioset)">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>CHECK abstract: removed from output (invalid location in 5.0).</xsl:text>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select="." mode="copy"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="indexterm">
  <!-- don't copy the defaulted significance='normal' attribute -->
  <indexterm>
    <xsl:call-template name="copy.attributes">
      <xsl:with-param name="suppress">
	<xsl:if test="@significance = 'normal'">significance</xsl:if>
      </xsl:with-param>
    </xsl:call-template>
    <xsl:apply-templates/>
  </indexterm>
</xsl:template>

<xsl:template match="ackno" priority="200">
  <acknowledgements>
    <xsl:copy-of select="@*"/>
    <para>
      <xsl:apply-templates/>
    </para>
  </acknowledgements>
</xsl:template>

<xsl:template match="lot|lotentry|tocback|tocchap|tocfront|toclevel1|
		     toclevel2|toclevel3|toclevel4|toclevel5|tocpart" priority="200">
  <tocdiv>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates/>
  </tocdiv>
</xsl:template>

<xsl:template match="action" priority="200">
  <phrase remap="action">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </phrase>
</xsl:template>

<xsl:template match="beginpage" priority="200">
  <xsl:comment> beginpage pagenum=<xsl:value-of select="@pagenum"/> </xsl:comment>
  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>Replacing beginpage with comment</xsl:text>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

<xsl:template match="structname|structfield" priority="200">
  <varname remap="{local-name(.)}">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </varname>
</xsl:template>
  
<!-- ====================================================================== -->
<!-- glossterm and term have broader content models in 4.x than 5.0.
     Warn when an unsupported element is found under glossterm.
     Because the synopsis elements can contain things that phrase cannot,
     leave them as is and warn.
     For other elements, change them into phrase recursively and lose attributes.
-->
<xsl:template match="glossterm|term">
  <xsl:element name="{local-name(.)}">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates mode="clean-terms"/>
  </xsl:element>
</xsl:template>

<!-- Any other elements inside term or glossterm which doesn't have a
     template rule are copied 
-->
<xsl:template match="*" mode="clean-terms">
    <xsl:apply-templates select="." mode="copy"/>
</xsl:template>
  

<!-- The synopsis elements have child elements that don't work inside phrase, plus
     they have attributes that shouldn't be lost. So, leave as is, but warn. -->
<xsl:template match="classsynopsis|cmdsynopsis|constructorsynopsis
                     |destructorsynopsis|fieldsynopsis|methodsynopsis|synopsis" mode="clean-terms">
  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>CHECK OUTPUT: Found </xsl:text><xsl:value-of select="local-name(.)"/>
      <xsl:text> inside </xsl:text><xsl:value-of select="local-name(..)"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:element name="{local-name(.)}">
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </xsl:element>
</xsl:template>

<!-- The following elements probably can be safely turned into phrase recursively -->
<xsl:template match="authorinitials|corpcredit|interface|medialabel|othercredit|revhistory" mode="clean-terms">
  <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>Replacing </xsl:text><xsl:value-of select="local-name(.)"/>
      <xsl:text> inside </xsl:text><xsl:value-of select="local-name(..)"/>
      <xsl:text> with phrase.</xsl:text>
    </xsl:with-param>
  </xsl:call-template>
  <phrase remap="{local-name(.)}">
    <!-- Don't copy attributes -->
    <xsl:apply-templates mode="make-phrase"/>
  </phrase>
</xsl:template>

<!-- affiliation can appear in a much smaller number of elements in 5.0. But, it contains
     elements that cannot appear in a phrase. So, replace all child elements, recursively,
     with <phrase remap="element name"...  Don't keep attributes, which won't work on phrase. -->

<xsl:template match="affiliation[not(parent::author) and not(parent::collab) and not(parent::editor) and not(parent::org) and not(parent::othercredit) and not(parent::person)]">
    <xsl:call-template name="emit-message">
    <xsl:with-param name="message">
      <xsl:text>CHECK OUTPUT: Converting </xsl:text><xsl:value-of select="local-name(.)"/>
      <xsl:text> to phrase in </xsl:text><xsl:value-of select="local-name(..)"/>
    </xsl:with-param>
  </xsl:call-template>
  <phrase remap="{local-name(.)}">
    <!-- Don't copy attributes -->
    <xsl:apply-templates mode="make-phrase"/>
  </phrase>
</xsl:template>

<!-- This template recursively changes an element with remap="name of element".
     Does this recursively through children. -->
<xsl:template match="*" mode="make-phrase">
  <phrase remap="{local-name(.)}">
    <!-- Don't copy attributes -->
    <xsl:apply-templates mode="make-phrase"/>
  </phrase>
</xsl:template>

<!-- ====================================================================== -->

<!-- 6 Feb 2008, ndw changed mode=copy so that it only copies the first level,
     then it switches back to "normal" mode so that other rewriting templates
     catch embedded fixes -->

<!--
<xsl:template match="ulink" priority="200" mode="copy">
  <xsl:choose>
    <xsl:when test="node()">
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Converting ulink to phrase.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>

      <phrase xlink:href="{@url}">
	<xsl:call-template name="copy.attributes">
	  <xsl:with-param name="suppress" select="'url'"/>
	</xsl:call-template>
	<xsl:apply-templates/>
      </phrase>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="emit-message">
        <xsl:with-param name="message">
          <xsl:text>Converting ulink to uri.</xsl:text>
        </xsl:with-param>
      </xsl:call-template>

      <uri xlink:href="{@url}">
	<xsl:call-template name="copy.attributes">
	  <xsl:with-param name="suppress" select="'url'"/>
	</xsl:call-template>
	<xsl:value-of select="@url"/>
      </uri>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="sgmltag" priority="200" mode="copy">
  <tag>
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </tag>
</xsl:template>
-->

<xsl:template match="*" mode="copy">
  <xsl:copy>
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<!--
<xsl:template match="comment()|processing-instruction()|text()" mode="copy">
  <xsl:copy/>
</xsl:template>
-->

<!-- ====================================================================== -->

<xsl:template match="*">
  <xsl:copy>
    <xsl:call-template name="copy.attributes"/>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

<xsl:template match="comment()|processing-instruction()|text()">
  <xsl:copy/>
</xsl:template>

<!-- ====================================================================== -->

<xsl:template name="copy.attributes">
  <xsl:param name="src" select="."/>
  <xsl:param name="suppress" select="''"/>
  <xsl:param name="suppress.default" select="''"/>

  <xsl:for-each select="$src/@*">
    <xsl:variable name="suppressed.value">
      <xsl:choose>
	<xsl:when test="not(contains($suppress.default, concat(local-name(.),'=')))">
	  <xsl:text>this-value-never-matches</xsl:text>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:value-of select="substring-before(substring-after(concat($suppress.default,' '), concat(local-name(.),'=')),' ')"/>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="local-name(.) = 'moreinfo'">
        <xsl:call-template name="emit-message">
          <xsl:with-param name="message">
            <xsl:text>Discarding moreinfo on </xsl:text>
            <xsl:value-of select="local-name($src)"/>
          </xsl:with-param>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="local-name(.) = 'lang'">
        <xsl:attribute name="xml:lang">
          <xsl:value-of select="."/>
        </xsl:attribute>
      </xsl:when>
      <xsl:when test="local-name(.) = 'id'">
        <xsl:attribute name="xml:id">
          <xsl:value-of select="."/>
        </xsl:attribute>
      </xsl:when>
      <xsl:when test="$suppress = local-name(.)"/>
      <xsl:when test=". = $suppressed.value"/>
      <xsl:when test="local-name(.) = 'float'">
	<xsl:choose>
	  <xsl:when test=". = '1'">
            <xsl:call-template name="emit-message">
              <xsl:with-param name="message">
                <xsl:text>Discarding float on </xsl:text>
                <xsl:value-of select="local-name($src)"/>
              </xsl:with-param>
            </xsl:call-template>
            <xsl:if test="not($src/@floatstyle)">
	      <xsl:call-template name="emit-message">
                <xsl:with-param name="message">
                  <xsl:text>Adding floatstyle='normal' on </xsl:text>
                  <xsl:value-of select="local-name($src)"/>
                </xsl:with-param>
              </xsl:call-template>
              <xsl:attribute name="floatstyle">
                <xsl:text>normal</xsl:text>
	      </xsl:attribute>
	    </xsl:if>
	  </xsl:when>
	  <xsl:when test=". = '0'">
	    <xsl:call-template name="emit-message">
              <xsl:with-param name="message">
                <xsl:text>Discarding float on </xsl:text>
                <xsl:value-of select="local-name($src)"/>
              </xsl:with-param>
            </xsl:call-template>
          </xsl:when>
	  <xsl:otherwise>
	    <xsl:call-template name="emit-message">
          <xsl:with-param name="message">
            <xsl:text>Discarding float on </xsl:text>
            <xsl:value-of select="local-name($src)"/>
          </xsl:with-param>
            </xsl:call-template>
            <xsl:if test="not($src/@floatstyle)">
              <xsl:call-template name="emit-message">
                <xsl:with-param name="message">
                  <xsl:text>Adding floatstyle='</xsl:text>
                  <xsl:value-of select="."/>
                  <xsl:text>' on </xsl:text>
                  <xsl:value-of select="local-name($src)"/>
                </xsl:with-param>
              </xsl:call-template>
              <xsl:attribute name="floatstyle">
		<xsl:value-of select="."/>
	      </xsl:attribute>
	    </xsl:if>
	  </xsl:otherwise>
	</xsl:choose>
      </xsl:when>
      <xsl:when test="local-name(.) = 'entityref'">
	<xsl:attribute name="fileref">
	  <xsl:value-of select="unparsed-entity-uri(@entityref)"/>
	</xsl:attribute>
      </xsl:when>

      <xsl:when test="local-name($src) = 'simplemsgentry'
	              and local-name(.) = 'audience'">
        <xsl:attribute name="msgaud">
          <xsl:value-of select="."/>
        </xsl:attribute>
      </xsl:when>
      <xsl:when test="local-name($src) = 'simplemsgentry'
	              and local-name(.) = 'origin'">
        <xsl:attribute name="msgorig">
          <xsl:value-of select="."/>
        </xsl:attribute>
      </xsl:when>
      <xsl:when test="local-name($src) = 'simplemsgentry'
	              and local-name(.) = 'level'">
        <xsl:attribute name="msglevel">
          <xsl:value-of select="."/>
        </xsl:attribute>
      </xsl:when>

      <!-- * for upgrading XSL litprog params documentation -->
      <xsl:when test="local-name($src) = 'refmiscinfo'
                      and local-name(.) = 'role'
                      and . = 'type'
                      ">
        <xsl:call-template name="emit-message">
          <xsl:with-param name="message">
            <xsl:text>Converting refmiscinfo@role=type to </xsl:text>
            <xsl:text>@class=other,otherclass=type</xsl:text>
          </xsl:with-param>
        </xsl:call-template>
        <xsl:attribute name="class">other</xsl:attribute>
        <xsl:attribute name="otherclass">type</xsl:attribute>
      </xsl:when>

      <xsl:otherwise>
        <xsl:copy/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>

<!-- ====================================================================== -->

<xsl:template match="*" mode="addNS">
  <xsl:choose>
    <xsl:when test="namespace-uri(.) = ''">
      <xsl:element name="{local-name(.)}"
		   namespace="http://docbook.org/ns/docbook">
	<xsl:if test="not(ancestor::*[namespace-uri(.)=''])">
	  <xsl:attribute name="version"><xsl:value-of select="$db5.version.string"/></xsl:attribute>
	</xsl:if>
	<xsl:copy-of select="@*"/>
	<xsl:apply-templates mode="addNS"/>
      </xsl:element>
    </xsl:when>
    <xsl:otherwise>
      <xsl:copy>
      <xsl:if test="namespace-uri(.) = 'http://docbook.org/ns/docbook' and
	      not(ancestor::*[namespace-uri(.)='http://docbook.org/ns/docbook'])">
	  <xsl:attribute name="version"><xsl:value-of select="$db5.version.string"/></xsl:attribute>
	</xsl:if>
	<xsl:copy-of select="@*"/>
	<xsl:apply-templates mode="addNS"/>
      </xsl:copy>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="comment()|processing-instruction()|text()" mode="addNS">
  <xsl:copy/>
</xsl:template>

<!-- ====================================================================== -->

<xsl:template name="emit-message">
  <xsl:param name="message"/>
  <xsl:message>
    <xsl:value-of select="$message"/>
    <xsl:text> (</xsl:text>
    <xsl:value-of select="$rootid"/>
    <xsl:text>)</xsl:text>
  </xsl:message>
</xsl:template>

</xsl:stylesheet>
