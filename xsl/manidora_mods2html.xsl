<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:php="http://php.net/xsl" version="1.0">
  <xsl:param name="islandoraUrl"/>
  <xsl:param name="collections"/>
  <xsl:variable name="smallcase">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="uppercase">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>

  <xsl:param name="searchUrl">/islandora/search/</xsl:param>

  <!-- XXX: Should probably be tied to some config in the front-end. -->
  <xsl:param name="type_of_resource_field">type_of_resource_mt</xsl:param>
  <xsl:param name="subject_name_field">subject_name_mt</xsl:param>
  <xsl:param name="subject_topic_field">subject_topic_mt</xsl:param>
  <xsl:param name="subject_title_field">subject_title_mt</xsl:param>
  <xsl:param name="language_field">language_mt</xsl:param>
  <xsl:param name="member_field">RELS_EXT_isMemberOfCollection_uri_ms</xsl:param>
  <xsl:param name="related_field">related_item_title_mt</xsl:param>

  
  <xsl:template match="mods:mods">
    <table class="manidora-metadata">
        <tr>
            <td class="label">Title</td>
            <td><xsl:value-of select="mods:titleInfo/mods:title"/></td>
        </tr>
        <xsl:if test="string-length($collections) &gt; 0">
          <tr>
            <td class="label">Collections</td>
            <td><xsl:copy-of select="php:functionString('manidora_return_collection_nodeset', $collections)"/></td>
          </tr>
        </xsl:if>
        <!--<xsl:apply-templates select="mods:relatedItem"></xsl:apply-templates>-->
        <xsl:apply-templates select="mods:abstract"></xsl:apply-templates>
        <!--<xsl:apply-templates select="mods:name"></xsl:apply-templates>-->
        <xsl:apply-templates select="mods:typeOfResource"></xsl:apply-templates>
        <xsl:apply-templates select="mods:subject/mods:temporal"></xsl:apply-templates>
        <xsl:apply-templates select="mods:subject"></xsl:apply-templates>
        <xsl:apply-templates select="mods:subject/mods:hierarchicalGeographic"></xsl:apply-templates>
        <xsl:apply-templates select="mods:language"></xsl:apply-templates>
        <xsl:apply-templates select="mods:note[@type!=&quot;cid&quot; and @type != &quot;objectID&quot; and @type != &quot;imageID&quot;]"></xsl:apply-templates>
        <xsl:apply-templates select="mods:location/mods:physicalLocation"></xsl:apply-templates>
        <xsl:apply-templates select="mods:location/mods:shelfLocator"></xsl:apply-templates>
        <xsl:apply-templates select="mods:physicalDescription/mods:internetMediaType"></xsl:apply-templates>
        <xsl:apply-templates select="mods:identifier[@type=&quot;local&quot;]"></xsl:apply-templates>
        <xsl:apply-templates select="mods:relatedItem/mods:location/mods:url" />
        <xsl:apply-templates select="mods:identifier[@type=&quot;hdl&quot;]"></xsl:apply-templates>        
        
        <xsl:apply-templates select="mods:accessCondition"></xsl:apply-templates><!-- Copyright -->

        <xsl:apply-templates select="mods:relatedItem/mods:part/mods:detail"></xsl:apply-templates>
        <xsl:apply-templates select="mods:relatedItem/mods:part/mods:extent/mods:start"></xsl:apply-templates>
        <xsl:apply-templates select="mods:originInfo/mods:issuance"></xsl:apply-templates>
        <xsl:apply-templates select="mods:originInfo/mods:frequency"></xsl:apply-templates>
        <xsl:apply-templates select="mods:relatedItem/mods:part/mods:date"></xsl:apply-templates>
    </table>
  </xsl:template>
  
  <!-- BASIC OUTPUT TEMPLATE -->
  <xsl:template name="basic_output">
      <xsl:param name="label"/>
      <xsl:param name="content" />
      <tr>
          <td class="label"><xsl:value-of select="$label"/>:</td>
          <td><xsl:copy-of select="$content"/></td>
      </tr>
  </xsl:template>
  <!-- BASIC OUTPUT TEMPLATE -->
 

  <xsl:template mode="search_link" match="text()">
    <xsl:param name="field"/>
    <xsl:variable name="textValue" select="normalize-space(.)"/>

    <xsl:attribute name="href">
      <xsl:value-of select="$islandoraUrl"/>
      <xsl:value-of select="$searchUrl"/>
      <xsl:value-of select="$field"/>
      <xsl:text>%3A%22</xsl:text>
      <xsl:value-of select="$textValue"/>
      <xsl:text>%22</xsl:text>
    </xsl:attribute>
  </xsl:template>
  
  <xsl:template mode="search_link2" match="text()">
    <xsl:param name="field"/>
    <xsl:variable name="textValue" select="normalize-space(.)"/>
    
    <a>
        <xsl:attribute name="target">_parent</xsl:attribute>
        <xsl:attribute name="href">
            <xsl:value-of select="$islandoraUrl"/>
          <xsl:value-of select="$searchUrl"/>
          <xsl:value-of select="$field"/>
          <xsl:text>%3A%22</xsl:text>
          <xsl:value-of select="$textValue"/>
          <xsl:text>%22</xsl:text>
        </xsl:attribute>
        <xsl:value-of select="."/>
    </a>
  </xsl:template>

  <xsl:template match="mods:typeOfResource">
      <xsl:call-template name="basic_output">
          <xsl:with-param name="label">Format</xsl:with-param>
          <xsl:with-param name="content"><xsl:apply-templates mode="search_link2" match="text()">
              <xsl:with-param name="field" select="$type_of_resource_field"/>
          </xsl:apply-templates></xsl:with-param>
      </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:name">
    <xsl:choose>
      <xsl:when test="@type = &apos;organization&apos;">
        <li>
          <strong>
            <xsl:apply-templates select="mods:role/mods:roleTerm"></xsl:apply-templates> - Organization: </strong>
          <xsl:value-of select="mods:namePart"></xsl:value-of>
        </li>
      </xsl:when>
      <xsl:when test="@type = &apos;personal&apos;">
        <li>
          <strong>
            <xsl:apply-templates select="mods:role/mods:roleTerm"></xsl:apply-templates> - Personal: </strong>
          <xsl:value-of select="mods:namePart"></xsl:value-of>
          <xsl:if test="mods:namePart[@type=&apos;date&apos;]">
            <xsl:value-of select="concat(&apos; &apos;,mods:namePart[@type=&apos;date&apos;])"></xsl:value-of>
          </xsl:if>
        </li>
      </xsl:when>
      <xsl:when test="@type = &apos;conference&apos;">
        <li>
          <strong>
            <xsl:apply-templates select="mods:role/mods:roleTerm"></xsl:apply-templates> - Conference: </strong>
          <xsl:for-each select="mods:namePart">
            <xsl:value-of select="concat(text(),&apos; &apos;)"></xsl:value-of>
          </xsl:for-each>
        </li>
      </xsl:when>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="mods:relatedItem[@type = &apos;host&apos; and mods:titleInfo/mods:title]">
      <xsl:call-template name="basic_output">
        <xsl:with-param name="label">Collection</xsl:with-param>
        <xsl:with-param name="content"><a>
              <xsl:attribute name="href">
                  <xsl:choose>
                      <xsl:when test="mods:identifier">
                          <xsl:value-of select="concat($islandoraUrl, $searchUrl, &apos;?f%5B0%5D=&apos;, $member_field, &apos;%3A%22&apos;,normalize-space(mods:identifier), &apos;%22&apos;)"></xsl:value-of>
                      </xsl:when>
                      <xsl:otherwise>
                          <xsl:value-of select="concat($islandoraUrl, $searchUrl, &apos;?f%5B0%5D=&apos;, $related_field, &apos;%3A%22&apos;,normalize-space(mods:titleInfo/mods:title), &apos;%22&apos;)"></xsl:value-of>
                      </xsl:otherwise>
                  </xsl:choose>
              </xsl:attribute>
              <xsl:attribute name="target">_parent</xsl:attribute>
              <xsl:value-of select="mods:titleInfo/mods:title"/></a>
         </xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <!-- BASIC template output -->
  <xsl:template match="mods:relatedItem/mods:part/mods:detail[@type=&quot;volume&quot;]">
      <xsl:call-template name="basic_output">
          <xsl:with-param name="label">Volume</xsl:with-param>
          <xsl:with-param name="content"><xsl:value-of select="mods:number"/></xsl:with-param>
      </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:relatedItem/mods:part/mods:detail[@type=&quot;issue&quot;]">
    <xsl:call-template name="basic_output">
        <xsl:with-param name="label">Issue</xsl:with-param>
        <xsl:with-param name="content"><xsl:value-of select="mods:number"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>
      
  <xsl:template match="mods:abstract">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Description</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:relatedItem/mods:part/mods:extent/mods:start">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Page Start</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:relatedItem/mods:location/mods:url">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Source</xsl:with-param>
      <xsl:with-param name="content"><a><xsl:attribute name="href"><xsl:value-of select="text()"/></xsl:attribute><xsl:value-of select="text()"/></a></xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:relatedItem/mods:part/mods:date">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Date</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:originInfo/mods:issuance">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Issuance</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>  </xsl:template>
  
  <xsl:template match="mods:originInfo/mods:frequency">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Frequency</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:identifier[@type=&quot;local&quot;]">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Local Identifier</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:identifier[@type=&quot;hdl&quot;]">
      <xsl:call-template name="basic_output">
          <xsl:with-param name="label">Permalink</xsl:with-param>
          <xsl:with-param name="content"><a>
              <xsl:attribute name="target">_parent</xsl:attribute>
              <xsl:attribute name="href">http://hdl.handle.net/<xsl:value-of select="normalize-space(text())"/></xsl:attribute>
	          http://hdl.handle.net/<xsl:value-of select="normalize-space(text())"/></a></xsl:with-param>
	</xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:subject">
    <xsl:if test="mods:topic or mods:name[@type=&quot;personal&quot;] or mods:titleInfo">
      <xsl:call-template name="basic_output">
          <xsl:with-param name="label">Subjects</xsl:with-param>
          <xsl:with-param name="content">
            <xsl:for-each select="mods:topic">
              <xsl:apply-templates mode="search_link2" select="text()">
                <xsl:with-param name="field" select="$subject_topic_field"/>
              </xsl:apply-templates>
              <xsl:if test="position() &lt; last() or mods:name[@type=&quot;personal&quot;] or mods:titleInfo"><xsl:text>, </xsl:text></xsl:if>
            </xsl:for-each>
            <xsl:for-each select="mods:name[@type=&quot;personal&quot;]">
              <xsl:apply-templates mode="search_link2" select="mods:namePart/text()">
                <xsl:with-param name="field" select="$subject_name_field"/>
              </xsl:apply-templates>
              <xsl:if test="position() &lt; last() or mods:titleInfo"><xsl:text>, </xsl:text></xsl:if>
            </xsl:for-each>
            <xsl:for-each select="mods:titleInfo">
              <xsl:apply-templates mode="search_link2" select="mods:title/text()">
                <xsl:with-param name="field" select="$subject_title_field"/>
              </xsl:apply-templates>
              <xsl:if test="position() &lt; last()"><xsl:text>, </xsl:text></xsl:if>
            </xsl:for-each>
          </xsl:with-param>
        </xsl:call-template>
    </xsl:if>
<!--    <xsl:apply-templates mode="subject" select="mods:name[@type=&apos;personal&apos;]"/>
    <xsl:apply-templates mode="subjectTitle" select="mods:titleInfo"/>-->
  </xsl:template>
  <!-- Moved into above template -->
<!--  <xsl:template match="mods:titleInfo" mode="subjectTitle">
    <li>
      <strong>Subject - Title: </strong>
      <xsl:element name="a">
        <xsl:attribute name="target">_parent</xsl:attribute>
        <xsl:apply-templates mode="search_link" select="mods:title/text()">
          <xsl:with-param name="field" select="$subject_title_field"/>
        </xsl:apply-templates>
        <xsl:value-of select="mods:title"></xsl:value-of>
      </xsl:element>
    </li>
  </xsl:template>
  <xsl:template match="mods:name[@type = &apos;personal&apos;]" mode="subject">
    <li>
      <strong>Subject - Personal: </strong>
      <xsl:element name="a">
        <xsl:attribute name="target">_parent</xsl:attribute>
        <xsl:apply-templates mode="search_link" select="mods:namePart/text()">
          <xsl:with-param name="field" select="$subject_name_field"/>
        </xsl:apply-templates>
        <xsl:value-of select="mods:namePart"></xsl:value-of>
      </xsl:element>
    </li>
  </xsl:template>-->
  
  <xsl:template match="mods:hierarchicalGeographic">
    <xsl:if test="mods:country or mods:province or mods:city">
      <xsl:call-template name="basic_output">
        <xsl:with-param name="label">Place</xsl:with-param>
        <xsl:with-param name="content">
          <xsl:if test="mods:country">
            <xsl:value-of select="normalize-space(mods:country)"/>
            <xsl:if test="mods:province or mods:city">, </xsl:if>
          </xsl:if>
          <xsl:if test="mods:province">
            <xsl:value-of select="normalize-space(mods:province)"/>
            <xsl:if test="mods:city">, </xsl:if>
          </xsl:if>
          <xsl:value-of select="normalize-space(mods:city)"></xsl:value-of>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="mods:temporal">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Date</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:language">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Languages</xsl:with-param>
      <xsl:with-param name="content">
        <xsl:for-each select="mods:languageTerm">
          <xsl:apply-templates mode="search_link2" select="text()">
            <xsl:with-param name="field" select="$language_field"/>
          </xsl:apply-templates>
          <xsl:if test="position() &gt; last()">, </xsl:if>
        </xsl:for-each>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:physicalLocation">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Physical Location</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:shelfLocator">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Shelf Location</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:internetMediaType">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Original File MIME Type</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:accessCondition">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Copyright</xsl:with-param>
      <xsl:with-param name="content">
        <xsl:choose>
          <xsl:when test="normalize-space(@type) = &apos;Creative Commons License&apos;">
            <div class="manidora-commons-license">
              <a rel="license" target="_new">
                <xsl:attribute name="href"><xsl:value-of select="concat(&apos;http://creativecommons.org/licenses/&apos;,normalize-space(text()))"/></xsl:attribute>
                <img alt="Creative Commons License" style="border-width:0;">
                  <xsl:attribute name="src"><xsl:value-of select="concat(&apos;http://i.creativecommons.org/l/&apos;,normalize-space(text()),&apos;88x31.png&apos;)"/></xsl:attribute>
                </img>
              </a>
              This work is licensed under a <a rel="license" target="_new">
                <xsl:attribute name="href"><xsl:value-of select="concat(&apos;http://creativecommons.org/licenses/&apos;,normalize-space(text()))"/></xsl:attribute>
                Creative Commons License</a>
            </div>
          </xsl:when>
          <xsl:otherwise>
            <strong>
              <xsl:value-of select="concat(translate(substring(normalize-space(@type),1,1), $smallcase, $uppercase),substring(normalize-space(@type),2))"/>: </strong>
              <xsl:value-of select="text()"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <!--<xsl:template match="mods:note[@type != &quot;cid&quot; or @type != &quot;objectID&quot; or @type != &quot;imageID&quot;]">-->
  <xsl:template match="mods:note">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Note</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:role/mods:roleTerm">
    <xsl:choose>
      <xsl:when test="normalize-space(text()) = &apos;act&apos;">Actor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;adp&apos;">Adapter</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;anl&apos;">Analyst</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;anm&apos;">Animator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ann&apos;">Annotator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;app&apos;">Applicant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;arc&apos;">Architect</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;arr&apos;">Arranger</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;acp&apos;">Art copyist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;art&apos;">Artist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ard&apos;">Artistic director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;asg&apos;">Assignee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;asn&apos;">Associated name</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;att&apos;">Attributed name</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;auc&apos;">Auctioneer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;aut&apos;">Author</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;aqt&apos;">Author in quotations or text extracts</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;aft&apos;">Author of afterword, colophon, etc.</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;aud&apos;">Author of dialog</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;aui&apos;">Author of introduction, etc.</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;aus&apos;">Author of screenplay, etc.</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ant&apos;">Bibliographic antecedent</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;bnd&apos;">Binder</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;bdd&apos;">Binding designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;blw&apos;">Blurb writer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;bkd&apos;">Book designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;bkp&apos;">Book producer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;bjd&apos;">Bookjacket designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;bpd&apos;">Bookplate designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;bsl&apos;">Bookseller</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cll&apos;">Calligrapher</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ctg&apos;">Cartographer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cns&apos;">Censor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;chr&apos;">Choreographer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cng&apos;">Cinematographer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cli&apos;">Client</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;clb&apos;">Collaborator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;col&apos;">Collector</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;clt&apos;">Collotyper</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;clr&apos;">Colorist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cmm&apos;">Commentator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cwt&apos;">Commentator for written text</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;com&apos;">Compiler</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cpl&apos;">Complainant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cpt&apos;">Complainant-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cpe&apos;">Complainant-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cmp&apos;">Composer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cmt&apos;">Compositor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ccp&apos;">Conceptor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cnd&apos;">Conductor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;con&apos;">Conservator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;csl&apos;">Consultant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;csp&apos;">Consultant to a project</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cos&apos;">Contestant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cot&apos;">Contestant-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;coe&apos;">Contestant-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cts&apos;">Contestee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ctt&apos;">Contestee-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cte&apos;">Contestee-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ctr&apos;">Contractor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ctb&apos;">Contributor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cpc&apos;">Copyright claimant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cph&apos;">Copyright holder</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;crr&apos;">Corrector</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;crp&apos;">Correspondent</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cst&apos;">Costume designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cov&apos;">Cover designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cre&apos;">Creator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;cur&apos;">Curator of an exhibition</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dnc&apos;">Dancer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dtc&apos;">Data contributor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dtm&apos;">Data manager</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dte&apos;">Dedicatee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dto&apos;">Dedicator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dfd&apos;">Defendant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dft&apos;">Defendant-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dfe&apos;">Defendant-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dgg&apos;">Degree grantor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dln&apos;">Delineator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dpc&apos;">Depicted</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dpt&apos;">Depositor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dsr&apos;">Designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;drt&apos;">Director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dis&apos;">Dissertant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dbp&apos;">Distribution place</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dst&apos;">Distributor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dnr&apos;">Donor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;drm&apos;">Draftsman</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;dub&apos;">Dubious author</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;edt&apos;">Editor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;elg&apos;">Electrician</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;elt&apos;">Electrotyper</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;eng&apos;">Engineer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;egr&apos;">Engraver</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;etr&apos;">Etcher</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;evp&apos;">Event place</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;exp&apos;">Expert</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;fac&apos;">Facsimilist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;fld&apos;">Field director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;flm&apos;">Film editor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;fpy&apos;">First party</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;frg&apos;">Forger</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;fmo&apos;">Former owner</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;fnd&apos;">Funder</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;gis&apos;">Geographic information specialist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;hnr&apos;">Honoree</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;hst&apos;">Host</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ilu&apos;">Illuminator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ill&apos;">Illustrator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ins&apos;">Inscriber</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;itr&apos;">Instrumentalist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ive&apos;">Interviewee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ivr&apos;">Interviewer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;inv&apos;">Inventor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lbr&apos;">Laboratory</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ldr&apos;">Laboratory director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;led&apos;">Lead</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lsa&apos;">Landscape architect</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;len&apos;">Lender</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lil&apos;">Libelant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lit&apos;">Libelant-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lie&apos;">Libelant-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lel&apos;">Libelee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;let&apos;">Libelee-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lee&apos;">Libelee-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lbt&apos;">Librettist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lse&apos;">Licensee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lso&apos;">Licensor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lgd&apos;">Lighting designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ltg&apos;">Lithographer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;lyr&apos;">Lyricist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mfr&apos;">Manufacturer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mrb&apos;">Marbler</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mrk&apos;">Markup editor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mdc&apos;">Metadata contact</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mte&apos;">Metal-engraver</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mod&apos;">Moderator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mon&apos;">Monitor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mcp&apos;">Music copyist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;msd&apos;">Musical director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;mus&apos;">Musician</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;nrt&apos;">Narrator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;opn&apos;">Opponent</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;orm&apos;">Organizer of meeting</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;org&apos;">Originator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;oth&apos;">Other</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;own&apos;">Owner</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ppm&apos;">Papermaker</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pta&apos;">Patent applicant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pth&apos;">Patent holder</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pat&apos;">Patron</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;prf&apos;">Performer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pma&apos;">Permitting agency</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pht&apos;">Photographer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ptf&apos;">Plaintiff</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ptt&apos;">Plaintiff-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pte&apos;">Plaintiff-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;plt&apos;">Platemaker</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;prt&apos;">Printer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pop&apos;">Printer of plates</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;prm&apos;">Printmaker</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;prc&apos;">Process contact</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pro&apos;">Producer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pmm&apos;">Production manager</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;prd&apos;">Production personnel</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;prg&apos;">Programmer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pdr&apos;">Project director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pfr&apos;">Proofreader</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pup&apos;">Publication place</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pbl&apos;">Publisher</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;pbd&apos;">Publishing director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ppt&apos;">Puppeteer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rcp&apos;">Recipient</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rce&apos;">Recording engineer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;red&apos;">Redactor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ren&apos;">Renderer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rpt&apos;">Reporter</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rps&apos;">Repository</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rth&apos;">Research team head</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rtm&apos;">Research team member</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;res&apos;">Researcher</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rsp&apos;">Respondent</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rst&apos;">Respondent-appellant</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rse&apos;">Respondent-appellee</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rpy&apos;">Responsible party</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rsg&apos;">Restager</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rev&apos;">Reviewer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;rbr&apos;">Rubricator&quot;</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;sce&apos;">Scenarist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;sad&apos;">Scientific advisor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;scr&apos;">Scribe</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;scl&apos;">Sculptor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;spy&apos;">Second party</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;sec&apos;">Secretary</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;std&apos;">Set designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;sgn&apos;">Signer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;sng&apos;">Singer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;sds&apos;">Sound designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;spk&apos;">Speaker</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;spn&apos;">Sponsor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;stm&apos;">Stage manager</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;stn&apos;">Standards body</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;str&apos;">Stereotyper</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;stl&apos;">Storyteller</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;sht&apos;">Supporting host</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;srv&apos;">Surveyor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;tch&apos;">Teacher</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;tcd&apos;">Technical director</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;ths&apos;">Thesis advisor</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;trc&apos;">Transcriber</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;trl&apos;">Translator</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;tyd&apos;">Type designer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;tyg&apos;">Typographer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;uvp&apos;">University place</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;vdg&apos;">Videographer</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;voc&apos;">Vocalist</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;wit&apos;">Witness</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;wde&apos;">Wood-engraver</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;wdc&apos;">Woodcutter</xsl:when>
      <xsl:when test="normalize-space(text()) = &apos;wam&apos;">Writer of accompanying material</xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="normalize-space(text())"></xsl:value-of>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Delete text which is not explicitly output. -->
  <xsl:template match="text()"/>
  <xsl:template match="text()" mode="subject"/>
  <xsl:template match="text()" mode="subjectTitle"/>
</xsl:stylesheet>

