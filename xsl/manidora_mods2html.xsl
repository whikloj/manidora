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
  
  <xsl:key name="nameKeys" match="mods:name" use="mods:role/mods:roleTerm" />
  
  <xsl:key name="CCAccessKey" match="mods:accessCondition" use="concat(@type,'+',text())" />
  
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
        <xsl:apply-templates select="mods:abstract" />
        
        <xsl:apply-templates select="mods:typeOfResource" />
        <xsl:apply-templates select="mods:subject/mods:temporal" />
        <xsl:apply-templates select="mods:subject" />
        <xsl:apply-templates select="mods:subject/mods:hierarchicalGeographic" />
        <!--<xsl:apply-templates select="mods:name" />--><!-- 2013-09-04 (whikloj) : Trying to group Author/Creators/etc. -->
        <xsl:call-template name="groupNames" />
        <xsl:apply-templates select="mods:language" />
        <xsl:apply-templates select="mods:note[not(@type ='cid') and not(@type = 'objectID') and not(@type = 'imageID')]" />
        <xsl:apply-templates select="mods:location/mods:physicalLocation" />
        <xsl:apply-templates select="mods:location/mods:shelfLocator" />
        <xsl:apply-templates select="mods:physicalDescription/mods:internetMediaType" />
        <xsl:apply-templates select="mods:identifier[@type='local']" />
        <xsl:apply-templates select="mods:relatedItem/mods:location/mods:url" />
        <xsl:apply-templates select="mods:identifier[@type='hdl']" />
        <xsl:apply-templates select="mods:originInfo"/>
        <xsl:apply-templates select="mods:accessCondition" /><!-- Copyright -->

        <xsl:apply-templates select="mods:relatedItem/mods:part/mods:detail" />
        <xsl:apply-templates select="mods:relatedItem/mods:part/mods:extent/mods:start" />
        <xsl:apply-templates select="mods:relatedItem/mods:part/mods:date" />
    </table>
  </xsl:template>
  
  <!-- BASIC OUTPUT TEMPLATE -->
  <xsl:template name="basic_output">
    <xsl:param name="label"/>
    <xsl:param name="content" />
    <xsl:if test="string-length($label) &gt; 0 and string-length($content) &gt; 0">
      <tr>
          <td class="label"><xsl:value-of select="$label"/>:</td>
          <td><xsl:copy-of select="$content"/></td>
      </tr>
    </xsl:if>
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
  
  <xsl:template name="searchLink">
    <xsl:param name="field"/>
    <xsl:param name="searchVal"/>
    <xsl:param name="textVal" select="$searchVal"/>
    <a>
      <xsl:attribute name="target">_parent</xsl:attribute>
      <xsl:attribute name="href"><xsl:value-of select="concat($islandoraUrl,$searchUrl, $field, '%3A%22',normalize-space($searchVal),'%22')"/></xsl:attribute>
      <xsl:value-of select="$textVal"/>
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


  <xsl:template name="groupNames">
    <xsl:for-each select="//mods:name">
      <xsl:choose>
        <xsl:when test="count(key('nameKeys',descendant-or-self::mods:roleTerm[1])) &gt; 1">
          <xsl:if test="not(./preceding-sibling::node()//mods:roleTerm = descendant-or-self::mods:roleTerm[1])">
            <xsl:call-template name="basic_output">
              <xsl:with-param name="label">Names</xsl:with-param>
              <xsl:with-param name="content">
                <xsl:apply-templates select="key('nameKeys',descendant-or-self::mods:roleTerm[1])" mode="grouping"/>
              </xsl:with-param>
            </xsl:call-template>
        </xsl:if>
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="." mode="basic"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>
          
  <xsl:template match="mods:name" mode="basic">
    <xsl:if test="string-length(text()) &gt; 0">
      <xsl:call-template name="basic_output">
        <xsl:with-param name="label">Name</xsl:with-param>
        <xsl:with-param name="content">
          <xsl:apply-templates select="." mode="text_out" />
          <xsl:text> </xsl:text>
          <xsl:apply-templates select="descendant-or-self::mods:roleTerm[1]"/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>
  
  <xsl:template match="mods:name" mode="grouping">
    <xsl:apply-templates select="." mode="text_out" />
    <xsl:text> </xsl:text>
    <xsl:apply-templates select="descendant-or-self::mods:roleTerm[1]"/><br />
  </xsl:template>
  
  <xsl:template match="mods:name" mode="text_out">
    <!-- Output nameParts as text -->
    <xsl:choose>
      <xsl:when test="mods:namePart[@type='family'] and mods:namePart[@type='given']">
        <xsl:value-of select="mods:namePart[@type='family']"/>
        <xsl:text>, </xsl:text>
        <xsl:if test="mods:namePart[@type='termsOfAddress']"><xsl:value-of select="mods:namePart[@type='termsOfAddress']"/><xsl:text> </xsl:text></xsl:if>
        <xsl:value-of select="mods:namePart[@type='given']"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:for-each select="mods:namePart">
          <xsl:value-of select="text()" />
          <xsl:if test="position() &lt; last()">
            <xsl:text> </xsl:text>
          </xsl:if>
        </xsl:for-each>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="mods:relatedItem[@type = 'host' and mods:titleInfo/mods:title]">
      <xsl:call-template name="basic_output">
        <xsl:with-param name="label">Collection</xsl:with-param>
        <xsl:with-param name="content"><a>
              <xsl:attribute name="href">
                  <xsl:choose>
                      <xsl:when test="mods:identifier">
                          <xsl:value-of select="concat($islandoraUrl, $searchUrl, '?f%5B0%5D=', $member_field, '%3A%22',normalize-space(mods:identifier), '%22')"></xsl:value-of>
                      </xsl:when>
                      <xsl:otherwise>
                          <xsl:value-of select="concat($islandoraUrl, $searchUrl, '?f%5B0%5D=', $related_field, '%3A%22',normalize-space(mods:titleInfo/mods:title), '%22')"></xsl:value-of>
                      </xsl:otherwise>
                  </xsl:choose>
              </xsl:attribute>
              <xsl:attribute name="target">_parent</xsl:attribute>
              <xsl:value-of select="mods:titleInfo/mods:title"/></a>
         </xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <!-- BASIC template output -->
  <xsl:template match="mods:relatedItem/mods:part/mods:detail[@type='volume']">
      <xsl:call-template name="basic_output">
          <xsl:with-param name="label">Volume</xsl:with-param>
          <xsl:with-param name="content"><xsl:value-of select="mods:number"/></xsl:with-param>
      </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:relatedItem/mods:part/mods:detail[@type='issue']">
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
      <xsl:with-param name="content">
        <a><xsl:attribute name="href"><xsl:value-of select="text()"/></xsl:attribute>
        <xsl:choose>
          <xsl:when test="string-length(parent::node()/parent::node()/mods:titleInfo/mods:title) &gt; 0">
            <xsl:value-of select="../../mods:titleInfo/mods:title" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="text()"/>
          </xsl:otherwise>
        </xsl:choose>
        </a>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:relatedItem/mods:part/mods:date">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Date</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:originInfo">
    <xsl:apply-templates select="mods:publisher" />
    <xsl:apply-templates select="mods:issuedDate|mods:dateCreated" />
    <xsl:if test="mods:place[string-length(text()|*) &gt; 0]">
      <xsl:call-template name="basic_output">
        <xsl:with-param name="label">Publication location</xsl:with-param>
        <xsl:with-param name="content">
          <xsl:apply-templates select="mods:place/mods:placeTerm"><xsl:sort select="@authority" order="descending"/></xsl:apply-templates>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template match="mods:publisher">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Publisher</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:issuedDate|mods:dateCreated">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Publication date</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:placeTerm">
    <xsl:value-of select="text()"/>
    <xsl:if test="last() &gt; 1 and position() &lt; last()"><xsl:text>, </xsl:text></xsl:if>
    <xsl:text> </xsl:text>
  </xsl:template>

  <xsl:template match="mods:identifier[@type='local']">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Local Identifier</xsl:with-param>
      <xsl:with-param name="content"><xsl:value-of select="text()"/></xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="mods:identifier[@type='hdl']">
      <xsl:call-template name="basic_output">
          <xsl:with-param name="label">Permalink</xsl:with-param>
          <xsl:with-param name="content"><a>
              <xsl:attribute name="target">_parent</xsl:attribute>
              <xsl:attribute name="href">http://hdl.handle.net/<xsl:value-of select="normalize-space(text())"/></xsl:attribute>
	          http://hdl.handle.net/<xsl:value-of select="normalize-space(text())"/></a></xsl:with-param>
	</xsl:call-template>
  </xsl:template>
  
  <xsl:template match="mods:subject">
    <xsl:if test="mods:topic or mods:name[@type='personal'] or mods:titleInfo">
      <xsl:call-template name="basic_output">
          <xsl:with-param name="label">Subjects</xsl:with-param>
          <xsl:with-param name="content">
            <xsl:for-each select="mods:topic|mods:name[@type='personal']|mods:titleInfo">
              <xsl:comment><xsl:value-of select="name()"/></xsl:comment>
              <xsl:choose>
                <xsl:when test="name() = 'mods:topic' or name() = 'topic'">
                  <xsl:call-template name="searchLink">
                     <xsl:with-param name="field" select="$subject_topic_field"/>
                     <xsl:with-param name="searchVal" select="text()" />
                  </xsl:call-template>
                </xsl:when>
                <xsl:when test="name() = 'mods:name' or name() = 'name'">
                  <xsl:call-template name="searchLink">
                    <xsl:with-param name="field" select="$subject_name_field"/>
                    <xsl:with-param name="searchVal"><xsl:apply-templates select="." mode="text_out" /></xsl:with-param>
                    <xsl:with-param name="textVal"><xsl:apply-templates select="." mode="text_out" /></xsl:with-param>
                  </xsl:call-template>
                </xsl:when>
                <xsl:when test="name() = 'mods:title' or name() = 'title'">
                  <xsl:apply-templates mode="search_link2" select="mods:title/text()">
                    <xsl:with-param name="field" select="$subject_title_field"/>
                  </xsl:apply-templates>
                </xsl:when>
              </xsl:choose>
              <xsl:if test="position() &lt; last()"><xsl:text>; </xsl:text></xsl:if>
            </xsl:for-each>
<!--            <xsl:for-each select="mods:name[@type=&quot;personal&quot;]">
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
            </xsl:for-each>-->
          </xsl:with-param>
        </xsl:call-template>
    </xsl:if>
  </xsl:template>
  

  <xsl:template match="mods:hierarchicalGeographic">
    <xsl:if test="string-length(mods:country) &gt; 0 or string-length(mods:province) &gt; 0 or string-length(mods:city) &gt; 0">
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

  <xsl:template match="mods:accessCondition[not(generate-id() = generate-id(key('CCAccessKey', concat(@type, '+', text()))[1])) or count(key('CCAccessKey',concat(@type,'+',text()))) = 1]">
    <xsl:call-template name="basic_output">
      <xsl:with-param name="label">Copyright</xsl:with-param>
      <xsl:with-param name="content">
        <xsl:choose>
          <xsl:when test="normalize-space(@type) = 'Creative Commons License'">
            <div class="manidora-commons-license">
              <a rel="license" target="_new">
                <xsl:attribute name="href"><xsl:value-of select="concat('http://creativecommons.org/licenses/',normalize-space(text()))"/></xsl:attribute>
                <img alt="Creative Commons License" style="border-width:0;">
                  <xsl:attribute name="src"><xsl:value-of select="concat('http://i.creativecommons.org/l/',normalize-space(text()),'88x31.png')"/></xsl:attribute>
                </img>
              </a>
              This work is licensed under a <a rel="license" target="_new">
                <xsl:attribute name="href"><xsl:value-of select="concat('http://creativecommons.org/licenses/',normalize-space(text()))"/></xsl:attribute>
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

  <xsl:template match="mods:roleTerm">
    <xsl:choose>
      <xsl:when test="normalize-space(text()) = 'act'">(Actor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'adp'">(Adapter)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'anl'">(Analyst)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'anm'">(Animator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ann'">(Annotator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'app'">(Applicant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'arc'">(Architect)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'arr'">(Arranger)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'acp'">(Art copyist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'art'">(Artist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ard'">(Artistic director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'asg'">(Assignee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'asn'">(Associated name)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'att'">(Attributed name)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'auc'">(Auctioneer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'aut'">(Author)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'aqt'">(Author in quotations or text extracts)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'aft'">(Author of afterword, colophon, etc.)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'aud'">(Author of dialog)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'aui'">(Author of introduction, etc.)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'aus'">(Author of screenplay, etc.)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ant'">(Bibliographic antecedent)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'bnd'">(Binder)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'bdd'">(Binding designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'blw'">(Blurb writer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'bkd'">(Book designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'bkp'">(Book producer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'bjd'">(Bookjacket designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'bpd'">(Bookplate designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'bsl'">(Bookseller)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cll'">(Calligrapher)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ctg'">(Cartographer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cns'">(Censor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'chr'">(Choreographer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cng'">(Cinematographer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cli'">(Client)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'clb'">(Collaborator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'col'">(Collector)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'clt'">(Collotyper)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'clr'">(Colorist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cmm'">(Commentator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cwt'">(Commentator for written text)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'com'">(Compiler)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cpl'">(Complainant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cpt'">(Complainant-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cpe'">(Complainant-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cmp'">(Composer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cmt'">(Compositor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ccp'">(Conceptor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cnd'">(Conductor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'con'">(Conservator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'csl'">(Consultant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'csp'">(Consultant to a project)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cos'">(Contestant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cot'">(Contestant-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'coe'">(Contestant-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cts'">(Contestee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ctt'">(Contestee-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cte'">(Contestee-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ctr'">(Contractor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ctb'">(Contributor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cpc'">(Copyright claimant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cph'">(Copyright holder)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'crr'">(Corrector)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'crp'">(Correspondent)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cst'">(Costume designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cov'">(Cover designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cre'">(Creator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'cur'">(Curator of an exhibition)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dnc'">(Dancer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dtc'">(Data contributor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dtm'">(Data manager)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dte'">(Dedicatee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dto'">(Dedicator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dfd'">(Defendant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dft'">(Defendant-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dfe'">(Defendant-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dgg'">(Degree grantor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dln'">(Delineator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dpc'">(Depicted)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dpt'">(Depositor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dsr'">(Designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'drt'">(Director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dis'">(Dissertant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dbp'">(Distribution place)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dst'">(Distributor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dnr'">(Donor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'drm'">(Draftsman)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'dub'">(Dubious author)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'edt'">(Editor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'elg'">(Electrician)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'elt'">(Electrotyper)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'eng'">(Engineer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'egr'">(Engraver)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'etr'">(Etcher)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'evp'">(Event place)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'exp'">(Expert)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'fac'">(Facsimilist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'fld'">(Field director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'flm'">(Film editor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'fpy'">(First party)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'frg'">(Forger)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'fmo'">(Former owner)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'fnd'">(Funder)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'gis'">(Geographic information specialist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'hnr'">(Honoree)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'hst'">(Host)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ilu'">(Illuminator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ill'">(Illustrator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ins'">(Inscriber)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'itr'">(Instrumentalist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ive'">(Interviewee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ivr'">(Interviewer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'inv'">(Inventor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lbr'">(Laboratory)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ldr'">(Laboratory director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'led'">(Lead)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lsa'">(Landscape architect)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'len'">(Lender)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lil'">(Libelant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lit'">(Libelant-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lie'">(Libelant-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lel'">(Libelee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'let'">(Libelee-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lee'">(Libelee-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lbt'">(Librettist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lse'">(Licensee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lso'">(Licensor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lgd'">(Lighting designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ltg'">(Lithographer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'lyr'">(Lyricist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mfr'">(Manufacturer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mrb'">(Marbler)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mrk'">(Markup editor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mdc'">(Metadata contact)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mte'">(Metal-engraver)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mod'">(Moderator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mon'">(Monitor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mcp'">(Music copyist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'msd'">(Musical director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'mus'">(Musician)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'nrt'">(Narrator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'opn'">(Opponent)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'orm'">(Organizer of meeting)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'org'">(Originator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'oth'">(Other)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'own'">(Owner)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ppm'">(Papermaker)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pta'">(Patent applicant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pth'">(Patent holder)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pat'">(Patron)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'prf'">(Performer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pma'">(Permitting agency)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pht'">(Photographer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ptf'">(Plaintiff)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ptt'">(Plaintiff-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pte'">(Plaintiff-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'plt'">(Platemaker)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'prt'">(Printer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pop'">(Printer of plates)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'prm'">(Printmaker)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'prc'">(Process contact)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pro'">(Producer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pmm'">(Production manager)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'prd'">(Production personnel)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'prg'">(Programmer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pdr'">(Project director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pfr'">(Proofreader)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pup'">(Publication place)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pbl'">(Publisher)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'pbd'">(Publishing director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ppt'">(Puppeteer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rcp'">(Recipient)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rce'">(Recording engineer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'red'">(Redactor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ren'">(Renderer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rpt'">(Reporter)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rps'">(Repository)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rth'">(Research team head)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rtm'">(Research team member)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'res'">(Researcher)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rsp'">(Respondent)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rst'">(Respondent-appellant)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rse'">(Respondent-appellee)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rpy'">(Responsible party)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rsg'">(Restager)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rev'">(Reviewer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'rbr'">(Rubricator&quot;)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'sce'">(Scenarist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'sad'">(Scientific advisor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'scr'">(Scribe)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'scl'">(Sculptor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'spy'">(Second party)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'sec'">(Secretary)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'std'">(Set designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'sgn'">(Signer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'sng'">(Singer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'sds'">(Sound designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'spk'">(Speaker)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'spn'">(Sponsor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'stm'">(Stage manager)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'stn'">(Standards body)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'str'">(Stereotyper)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'stl'">(Storyteller)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'sht'">(Supporting host)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'srv'">(Surveyor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'tch'">(Teacher)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'tcd'">(Technical director)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'ths'">(Thesis advisor)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'trc'">(Transcriber)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'trl'">(Translator)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'tyd'">(Type designer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'tyg'">(Typographer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'uvp'">(University place)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'vdg'">(Videographer)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'voc'">(Vocalist)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'wit'">(Witness)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'wde'">Wood-engraver</xsl:when>
      <xsl:when test="normalize-space(text()) = 'wdc'">(Woodcutter)</xsl:when>
      <xsl:when test="normalize-space(text()) = 'wam'">(Writer of accompanying material)</xsl:when>
      <xsl:when test="string-length(normalize-space(text())) &gt; 0">
        <!-- not a code, so we assume full text -->
        <xsl:value-of select="normalize-space(concat('(',translate(substring(text(),1,1),$smallcase,$uppercase),translate(substring(text(),2),$uppercase,$smallcase),')'))" />
      </xsl:when>
    </xsl:choose>
  </xsl:template>

  <!-- Delete text which is not explicitly output. -->
  <xsl:template match="text()"/>
  <xsl:template match="text()" mode="subject"/>
  <xsl:template match="text()" mode="subjectTitle"/>
  <xsl:template match="node()" priority="-1"/>
</xsl:stylesheet>
