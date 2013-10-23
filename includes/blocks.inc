<?php


/**
 * @file
 * Contains functions for blocks.
 */
 
 /**
  * Implements hook_block_info().
  */
 function manidora_block_info() {
   $blocks['manidora_homepage_tabs'] = array(
     'info' => t('Manidora homepage tabs'),
   );
   $blocks['manidora_homepage_thumbnails'] = array(
     'info' => t('Manidora homepage thumbnails'),
   );
   return $blocks;
  }
  
  /**
   * Implements hook_block_view().
   */
  function manidora_block_view($delta = '') {
    $block = array();
    switch ($delta) {
      case 'manidora_homepage_tabs':
        $block['subject'] = 'Homepage Tabs';
        $block['content'] = __manidora_homepage_tabs_content();
        break;
      case 'manidora_homepage_thumbnails':
        $block['subject'] = 'Homepage Thumbnails';
        $block['content'] = __manidora_homepage_thumbnail_content();
        break;
    }
    return $block;
  }
  
  function __manidora_homepage_tabs_content(){
    /**
     * @todo Ideally the content for the Fedora derived tabs (departments, subjects, format) should be stored in a tab and administered through Drupal.
     *   Unfortunately, due to time-constraints we are hard-coding
     */
     module_load_include('php','islandora_solr','SolrPhpClient/Apache/Solr/Service');

     $subjects = array(
       "Aboriginal Peoples",
       "Arts, Music, and Architecture",
       "Books, Literature, and Language",
       "Business, Industry, and Labour",
       "Education History",
       "Engineering and Transportation",
       "Government, Law, and Human Rights",
       "Health and Medicine",
       "Immigration",
       "Media and Communications",
       "Northern Studies",
       "Prairie History",  
       "Religion, Philosophy, and Spiritualism",
       "Science and Technology",
       "Sports and Recreation",
       "University of Manitoba History",
       "War and the Military",
       "Women and Gender Studies",
       "World History, Peoples, and Cultures",
     );
     
     $formats = array(
       "Images" => array(
         "solr_field" => "type_of_resource_mt",
         "data" => array(
           "glass lantern slide",
           "photograph",
           "slide",
           "stereo photograph",
           "still image",
          ),
        ),
      "Text" => array(
        "solr_field" => "type_of_resource_mt",
        "data" => array(
          "text",
          "textual record",
        ),
      ),
      "Newspapers" => array(
       /* "solr_field" => "type_of_resource_mt",
        "data" => array(
          "newspaper",
        ),*/
        "url" => "islandora/object/uofm:libraries",
      ),
      "Books" => array(
        "solr_field" => "RELS_EXT_hasModel_uri_ms",
        "data" => array(
          "info:fedora%2Fislandora:bookCModel",
        ),
      ),
      "Moving images" => array(
        //"solr_field" => "type_of_resource_mt",
        "solr_field" => "dc.type",
        "data" => array(
          "moving image",
        ),
      ),
      "Sound recordings" => array(
        "solr_field" => "type_of_resource_mt",
        "data" => array(
          "sound recording",
          "sound recording - nonmusical",
        ),
      ),
      "Mixed material" => array(
        "solr_field" => "type_of_resource_mt",
        "data" => array(
          "mixed material",
        ),
      ),
    );
    
    $departments = array(
     // "Faculty of Architecture" => "",
      //"Faculty of Dentistry" => "uofm:dentalDAMS",
      "Faculty of Medicine Archives" => "uofm:medArchives",
      "Libraries: Archives & Special Collections" => "uofm:archives",
      "Libraries: Newspapers" => "uofm:libraries",
    );


    $output = <<<EOF
      <ul class="home-tabs-container">
      <li class="home-tab active"><a title="List Collections by Subject" id="home-tabs-subjects" class="home-tab-link" href="javascript:void(0)">Subjects</a></li>
      <li class="home-tab"><a title="List Collections Alphabetically" id="home-tabs-collections" class="home-tab-link" href="javascript:void(0)">Collections</a></li>
      <li class="home-tab"><a title="Browse by Format type" id="home-tabs-formats" class="home-tab-link" href="javascript:void(0)">Formats</a></li>
      <li class="home-tab"><a title="List Collections by UM Faculty or Department" id="home-tabs-depts" class="home-tab-link" href="javascript:void(0)">Departments</a></li>
      </ul>
      <div class="home-panel active" id="home-panel-subjects">
        <div class="column_50 first">
EOF;
        $half = intval(count($subjects) /2) + (count($subjects)%2);
        $count = 0;
      foreach ($subjects as $sub){
        if ($count == $half){
          $output .= "</div><!-- /.column_50 -->\n";
          $output .= "<div class=\"column_50 last\">\n";
        }
        $output .= l($sub,'islandora/search/subject_topic_facet_ms:"'.$sub.'"')."<br />\n";
        $count += 1;
      } 
      $output .=<<<EOF
        </div><!-- /#.column_50 -->
      </div><!-- /#home-panel-subjects -->
      <div class="home-panel" id="home-panel-collections">
EOF;

      $solr_query = '(RELS_EXT_isMemberOfCollection_uri_mt:"uofm:*" AND RELS_EXT_hasModel_uri_mt:("info:fedora/islandora:collectionCModel" "info:fedora/islandora:newspaperCModel"))';
      $solr_params = array("fl"=>"PID,dc.title",'fq'=>'-PID:("uofm:libraries" "uofm:architecture" "uofm:medArchives" "uofm:archives")');
      try{
        $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr')); 
        $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');

        $solr->setCreateDocuments(0);
        $results = $solr->search($solr_query,0,100,$solr_params);

        $tmpobjects = array();
        if ($results){
          $solr_results = json_decode($results->getRawResponse(), TRUE);
          foreach ($solr_results["response"]["docs"] as $key => $doc){
            $tmpobjects[] = array("PID"=>$doc['PID'], "title"=>$doc['dc.title'][0]);
          }
          // Solr can't sort by multivalued elements, so sort by title
          usort($tmpobjects,create_function('$a,$b','return strcasecmp($a["title"],$b["title"]);'));
          $objects = array();
          $cl = "";
          // Move the items so each letter that exists is an array of those collections 
          array_walk($tmpobjects, function($v,$k) use (&$objects,&$cl) { if (substr(strtolower(preg_replace('/^the\s?/i',"",$v['title'])),0,1) != $cl){ $cl = substr(strtolower(preg_replace('/^the\s?/i',"",$v['title'])),0,1);} if (array_key_exists($cl,$objects) && !is_array($objects[$cl])){ $objects[$cl] = array(); } $objects[$cl][] = $v; });      
          $tmp = "";
          foreach ($objects as $letter => $collections){
            $odd = array();
            $even = array();
            $both = array(&$even, &$odd);
            // Split items to even and odd array : http://stackoverflow.com/a/12405336
            array_walk($collections, function($v, $k) use ($both) { $both[$k % 2][] = $v; });  
            $both = array_merge($even,$odd);
            $tmp .= "<div class=\"coll_letter_box\"><a name=\"coll_$letter\" class=\"collection_spacer\"></a>";
            $tmp .= "<div class=\"column_50 first collections\">";
            $count = 0;
            foreach ($both as $c){
              if ($count == count($even)){
                $tmp .= "</div><!-- /.column_50 -->\n";
                $tmp .= "<div class=\"column_50 last\">\n";
              }
              $tmp .= l($c['title'],'islandora/object/' . $c['PID']) . "<br />\n";
              $count += 1;
            }
            $tmp .= "</div> <!-- /.column_50 -->\n";
            $tmp .= "</div> <!-- /.coll_letter_box -->\n";
          }

          $output .= "<span class=\"coll_alpha_list\">";
          for ($x=97; $x<122; $x+=1){
            if (array_key_exists(chr($x),$objects)){
              $output .= '<a href="#coll_' . chr($x) .'"> ' . chr($x) .' </a>|';
            } else {
              $output .= ' '.chr($x)." |";
            }
          }
          $output = substr($output,0,-3) . "</span>";
          $output .= $tmp;
        }
      } catch (Exception $e){
        watchdog('Manidora','Homepage blocks error, Solr query threw (%e)',array('%e'=> $e->getMessage()));
        $output .= "Unable to retrieve collections";
      }

      $output .=<<<EOF
      </div><!-- /#home-panel-collections -->
      <div class="home-panel" id="home-panel-formats">
        <div class="column_50 first">
EOF;
      foreach ($formats as $key => $format){
        $uri = "";
        if (array_key_exists('data',$format) && is_array($format['data']) && count($format['data'])>0){
          foreach ($format['data'] as $ele){
            $uri .= (!empty($uri)? " OR ":"") . '"'.$ele.'"';
          }
          $uri = $format['solr_field'].':('.$uri.')';
          if (!empty($uri)){
            $uri = l($key,"islandora/search/$uri",array('html'=>TRUE));
          }
        } else if (array_key_exists('url',$format)){
          $uri = l($key,$format['url']);
        }
        if (!empty($uri)){
          $output .= "$uri<br />\n";
        } else {
          $output .= "$key<br />\n";
        }
      }
      $output .=<<<EOF
        </div><!-- /.column_50 -->
      </div><!-- /#home-panel-formats -->
      <div class="home-panel" id="home-panel-depts">
        <div class="column_50 first">
EOF;
      foreach ($departments as $dept => $uri){
        if (!empty($uri)){
          $output .= l($dept,"islandora/object/$uri") . "<br />\n";
        } else {
          $output .= "$dept<br />\n";
        }
      }
      $output .=<<<EOF
        </div><!-- /.column_50 -->
      </div><!-- /#home-panel-depts -->
      <script type="text/javascript">
      <!--
      jQuery(document).ready(function(){
        jQuery('.home-tab-link').click(function(){
          jQuery('.home-tab, .home-panel').removeClass('active');
          jQuery('#home-panel-' + jQuery(this).attr('id').replace('home-tabs-','')).addClass('active');
          jQuery(this).parent('.home-tab').addClass('active');
        });
      });
      //-->
      </script>
EOF;

    return $output;

  }

/**
 * Display 16 thumbnails on the homepage from those selected
 */
function __manidora_homepage_thumbnail_content(){
  module_load_include('php','islandora_solr','SolrPhpClient/Apache/Solr/Service');
  $num_pics = 16; // How many thumbnails are we displaying
  $num_rows = 2; // Figure out when to break
  $thumbs = variable_get('manidora_thumbnail_pids'); // Get the pids from the variable
  $output = "";
  if (!is_null($thumbs)){ // If its null we display nothing.
    $t_array = explode('~',urldecode($thumbs));
    /** 
     * This is temporary for debugging, we extend the array by duplicating it to see the full selection.
     */
    if (count($t_array)<$num_pics){
      $dup = intval($num_pics / count($t_array)) + 1;
      $new_arr = array();
      for ($foo = 0; $foo < $dup; $foo += 1){
        $new_arr = array_merge($new_arr,$t_array);
      }
      $t_array = $new_arr;
    }
    /** 
     * End temporary extension
     */
    /**
     * We want to have the image title as mouseover in the block
     */
    $solr_query = '+PID:("'.implode('" "',$t_array).'")'; // Search for all the possible PIDs
    $solr_params = array('fl'=>'PID,dc.title'); // Get the PID and dc.title
    $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr')); 
    #$path_parts = parse_url('digitalcollections.lib.umanitoba.ca:8080/solr');
    $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');

    $solr->setCreateDocuments(0);
    $results = $solr->search($solr_query,0,100,$solr_params);
    if ($results){
      $solr_results = json_decode($results->getRawResponse(), TRUE);
      $data = array();
      foreach ($solr_results["response"]["docs"] as $key => $doc){
        $data[$doc['PID']] = $doc['dc.title'][0];
      }
    }
    if (count($t_array)>=$num_pics){
      $picked = array_rand($t_array,$num_pics);
      $output = '<div id="manidora-homepage-thumbs">';
      foreach ($picked as $pid){
        $title = '';
        // If $data is an array and has a key of the current PID, then we know the title
        if (isset($data) && is_array($data) && array_key_exists($t_array[$pid],$data)){
          $title = 'title="' . check_plain($data[$t_array[$pid]]) . '" ';
        }
        $output .= '<a '.$title.'href="' . url("islandora/object/" . $t_array[$pid]) .'" class="manidora-homepage-thumb-link"><img src="' . url("islandora/object/" . $t_array[$pid] . "/datastream/TN/view") . '" alt="Object '. check_plain($t_array[$pid]) .'" class="manidora-homepage-thumb-img" /></a>' . "\n";
      }
      $output .= "</div> <!-- /#manidora-homepage-thumbs -->\n";
    }
  }
  return $output;
}
    
  
