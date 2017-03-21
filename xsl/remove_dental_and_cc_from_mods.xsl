<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:mods="http://www.loc.gov/mods/v3">
    <!--
      Remove Dental and Creative Commons elements from the MODS form
    //-->
    <xsl:output encoding="UTF-8" indent="yes" media-type="text/xml" method="xml" version="1.0"/>
    <xsl:strip-space elements="*"/>
    <xsl:template match="*[not(node())]"/>

    <!-- Remove these subjects -->
    <xsl:template match="mods:subject[@displayLabel='title']" />
    <xsl:template match="mods:subject[@displayLabel='subject']"/>
    <xsl:template match="mods:subject[@displayLabel='fixed']"/>
    <xsl:template match="mods:subject[@displayLabel='removable']"/>
    <xsl:template match="mods:subject[@displayLabel='maxillofacial']"/>
    <xsl:template match="mods:subject[@displayLabel='implants']"/>
    <!-- Remove Creative Commons -->
    <xsl:template match="mods:accessCondition[@type='Creative Commons License'"]"/>

    <!-- Otherwise keep the non-empty elements -->
    <xsl:template match="node()|@*">
        <xsl:call-template name="copy"/>
    </xsl:template>
    <xsl:template name="copy">
        <xsl:copy>
            <xsl:apply-templates select="node()[normalize-space()]|@*[normalize-space()]"/>
        </xsl:copy>
    </xsl:template>

</xsl:stylesheet>