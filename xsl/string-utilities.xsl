<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:variable name="lowercase">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="uppercase">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>

  <xsl:template name="toLowerCase">
    <xsl:param name="text" />
    <xsl:value-of select="translate($text,$uppercase,$lowercase)"/>
  </xsl:template>

  <xsl:template name="toUpperCase">
    <xsl:param name="text" />
    <xsl:value-of select="translate($text,$lowercase,$uppercase)"/>
  </xsl:template>

  <xsl:template name="toProperCase">
    <xsl:param name="text" />
    <xsl:value-of select="concat(translate(substring($text, 1, 1), $lowercase, $uppercase), translate(substring($text, 2), $uppercase, $lowercase))" />
  </xsl:template>

  <!-- Renders text with <a href="">links</a> as actual hyperlinks.-->
  <xsl:template match="text()" mode="parseLinks">
    <xsl:call-template name="parseLinks">
      <xsl:with-param name="text" select="."/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="parseLinks">
    <xsl:param name="text" />
    <xsl:choose>
      <xsl:when test="contains($text, '&lt;a')">
        <!-- We found a link -->
        <xsl:value-of select="substring-before($text, '&lt;a')" />
        <xsl:call-template name="renderUrl">
          <xsl:with-param name="link" select="substring-after($text, '&lt;a')"/>
        </xsl:call-template>
        <xsl:call-template name="parseLinks">
          <xsl:with-param name="text" select="substring-after($text, '&lt;/a&gt;')"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$text"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- used internally by the parseLink mode above to render the hyperlink-->
  <xsl:template name="renderUrl">
    <xsl:param name="link"/>
    <xsl:variable name="afterHref">
      <xsl:call-template name="removeQuotes">
        <xsl:with-param name="text" select="substring-before(substring-after($link, 'href='), '&gt;')"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="linkText">
      <xsl:value-of select="substring-before(substring-after($link, '&gt;'), '&lt;/a&gt;')" />
    </xsl:variable>
    <xsl:element name="a">
      <xsl:attribute name="href"><xsl:value-of select="$afterHref"/></xsl:attribute>
      <xsl:value-of select="$linkText"/>
    </xsl:element>
  </xsl:template>

  <!-- Removes single and double quotes from text-->
  <xsl:template name="removeQuotes">
    <xsl:param name="text"/>
    <xsl:value-of select="translate(translate($text, '&quot;', ''), &quot;'&quot;, '')"/>
  </xsl:template>

</xsl:stylesheet>