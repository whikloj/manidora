<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                xmlns:mods="http://www.loc.gov/mods/v3"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <!-- This stylesheet does a clean up of MODS to ensure that all records have the mods: namespace
                    prefix in their records, this is required to insert the mods:identifier to the DAMs
           record in OAI-PMH -->
    <xsl:output method="xml" indent="yes" omit-xml-declaration="yes"/>
    <xsl:strip-space elements="*"/>
    <xsl:template match="/">
        <xsl:apply-templates select="*"/>
    </xsl:template>

    <!-- recreate all nodes and attributes with the mods: prefix -->
    <xsl:template match="node()">
        <xsl:element name="mods:{local-name()}" namespace="http://www.loc.gov/mods/v3">
            <xsl:copy-of select="@*"/>
            <xsl:apply-templates select="node()"/>
        </xsl:element>
    </xsl:template>

    <!-- Don't touch text -->
    <xsl:template match="text()" priority="0">
        <xsl:value-of select="."/>
    </xsl:template>

</xsl:stylesheet>