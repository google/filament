<?xml version='1.0'?> 
<xsl:stylesheet  
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"> 

<xsl:import href="http://docbook.sourceforge.net/release/xsl/current/xhtml/profile-docbook.xsl"/>

<xsl:param name="funcsynopsis.style">ansi</xsl:param>
<xsl:param name="citerefentry.link" select="'1'"></xsl:param>
<xsl:template name="generate.citerefentry.link"><xsl:value-of select="refentrytitle"/>.xml</xsl:template>

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
      <xsl:apply-templates select="."/>
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

</xsl:stylesheet>  
