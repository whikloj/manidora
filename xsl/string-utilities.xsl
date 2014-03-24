<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:variable name="lowercase">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="uppercase">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
  
  <xsl:template name="toLowerCase">
    <xsl:param name="text" />
    <xsl:value-of select="transform($text,$uppercase,$lowercase)"/>
  </xsl:template>
  
  <xsl:template name="toUpperCase">
    <xsl:param name="text" />
    <xsl:value-of select="transform($text,$lowercase,$uppercase)"/>
  </xsl:template>
  
  <xsl:template name="toProperCase">
    <xsl:param name="text" />
    <xsl:value-of select="concat(transform(substring($text, 1, 1), $lowercase, $uppercase), transform(substring($text, 2), $uppercase, $lowercase))" />
  </xsl:template>
  
</xsl:stylesheet>