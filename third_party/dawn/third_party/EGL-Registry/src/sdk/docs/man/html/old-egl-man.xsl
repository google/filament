<!-- The following is content that used to be in the EGL XSL directives
     now found in khronos-man.xsl. It is OpenCL formatting stuff done by
     Miller & Mattson and is being ignored for the moment. -->

    <!-- Set this param to a placeholder for the base URL of the
         external specification document. Include the beginning of the
         'namedest' function as well. See the script pageNumberLookup.rb
         to see how this placeholder gets replaced by the actual spec
         URL and target page number. This placeholder can be any string,
         and only needs to match the same placeholder string in
         pageNumberLookup.rb. -->
    <!-- This isn't used by EGL at present -->
    <xsl:param name="SpecBaseUrl">http://www.khronos.org/registry/egl/specs/eglTBD.pdf#namedest=</xsl:param>

    <!-- The following template creates the link for the Specification section  -->
    <!-- This isn't used by EGL at present -->
    <xsl:template match="olink">
        <xsl:text disable-output-escaping="yes">&lt;a href="</xsl:text>
        <xsl:value-of select="$SpecBaseUrl" />
        <xsl:value-of select="@uri" />
        <xsl:text disable-output-escaping="yes">" target="OpenCL Spec"&gt;</xsl:text>
        <xsl:value-of select="." />
        <xsl:text disable-output-escaping="yes">&lt;/a&gt;</xsl:text>
    </xsl:template>

    <!-- The following enables use of ulink for regular URLs on the pages-->
    <!-- This isn't used by EGL at present -->
    <xsl:template match="ulink">
        <xsl:text disable-output-escaping="yes">&lt;a href="</xsl:text>
        <xsl:value-of select="@url" />
        <xsl:text disable-output-escaping="yes">"&gt;</xsl:text>
        <xsl:value-of select="." />
        <xsl:text disable-output-escaping="yes">&lt;/a&gt;</xsl:text>
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

    <!-- This is somewhat redundant with the following template -->
    <xsl:template match="funcdef/replaceable">
        <xsl:call-template name="inline.italicseq"/>
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

    <!-- The templates funcprototype and paramdef are here so we can modify
         the layout of the synopsis so that it is not broken into so many
         columns in the table, control indenting, and more. -->

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

