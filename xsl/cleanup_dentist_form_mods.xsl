<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output encoding="UTF-8" indent="yes" media-type="text/xml" method="xml" version="1.0"/>
  <xsl:strip-space elements="*"/>
  <xsl:template match="*[not(node())]"/>
  <xsl:template match="node()|@*">
    <xsl:call-template name="copy"/>
  </xsl:template>
  <xsl:template name="copy">
    <xsl:copy>
      <xsl:apply-templates select="node()[normalize-space()]|@*[normalize-space()]"/>
    </xsl:copy>
  </xsl:template>
  <!-- Include the subject only if it's label is mentioned in the subject title -->
  <xsl:template name="include_subject">
    <xsl:if test="contains(../mods:subject[@displayLabel='title']/mods:topic, @displayLabel)">
      <xsl:call-template name="copy"/>
    </xsl:if>
  </xsl:template>
  <!-- Only include fixed if mentioned in the subject title. -->
  <xsl:template match="mods:subject[@displayLabel='fixed']">
    <xsl:call-template name="include_subject"/>
  </xsl:template>
  <!-- Only include removable if mentioned in the subject title. -->
  <xsl:template match="mods:subject[@displayLabel='removable']">
    <xsl:call-template name="include_subject"/>
  </xsl:template>
  <!-- Only include maxillofacial if mentioned in the subject title. -->
  <xsl:template match="mods:subject[@displayLabel='maxillofacial']">
    <xsl:call-template name="include_subject"/>
  </xsl:template>
  <!-- Only include implants if mentioned in the subject title. -->
  <xsl:template match="mods:subject[@displayLabel='implants']">
    <xsl:call-template name="include_subject"/>
  </xsl:template>
</xsl:stylesheet>
