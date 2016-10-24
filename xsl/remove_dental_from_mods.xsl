<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <xsl:output encoding="UTF-8" indent="yes" media-type="text/xml" method="xml" version="1.0"/>
    <xsl:strip-space elements="*"/>
    <xsl:template match="*[not(node())]"/>

    <!-- Remove these subjects -->
    <xsl:template match="mods:subject[@displayLabel='title' and text() = 'Anatomy']">
    </xsl:template>
    <xsl:template match="mods:subject[@displayLabel='subject' and text() = 'Examination']">
    </xsl:template>
    <!-- Only include removable if mentioned in the subject title. -->
    <xsl:template match="mods:subject[@displayLabel='fixed']">
    </xsl:template>
    <!-- Only include removable if mentioned in the subject title. -->
    <xsl:template match="mods:subject[@displayLabel='removable']">
    </xsl:template>
    <!-- Only include maxillofacial if mentioned in the subject title. -->
    <xsl:template match="mods:subject[@displayLabel='maxillofacial']">
    </xsl:template>
    <!-- Only include implants if mentioned in the subject title. -->
    <xsl:template match="mods:subject[@displayLabel='implants']">
    </xsl:template>

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