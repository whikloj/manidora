<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:pb="http://www.pbcore.org/PBCore/PBCoreNamespace.html" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
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
    <!-- Remove Creative Commons License -->
    <xsl:template match='pb:rightsLink[@annotation = "Creative Commons License"]' />

</xsl:stylesheet>
