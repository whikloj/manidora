<?php

/**
 * @file
 * Contains functions for blocks.
 */

/**
 * Implements hook_block_info().
 */
function manidora_block_info() {
  $blocks['manidora_simple_search'] = array(
    'info' => t('Manidora simple search'),
  );
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
    case 'manidora_simple_search':
      $block['subject'] = t('Manidora simple search');
      $block['content'] = drupal_get_form('manidora_simple_search_form');
      break;
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

/**
 * Manidora simple search form.
 *
 * Based on the Islandora Solr simple search block, this search box adds a
 * feature to search within the current collection or newspaper.
 */
function manidora_simple_search_form($form, &$form_state) {
  $params = $_GET;
  $hidden_filter = NULL;
  $rels_ext = array(
    'RELS_EXT_isMemberOfCollection_uri_ms',
    'RELS_EXT_isMemberOf_uri_ms',
  );
  // Find rels ext in parameters.
  if (isset($params['f'])) {
    foreach ($params['f'] as $filter) {
      $filter_arr = explode(':', $filter);
      foreach ($rels_ext as $rels) {
        if (strpos($filter_arr[0], $rels) === 0) {
          $hidden_filter = $filter;
          break;
        }
      }
    }
  }
  $options = array(
    0 => t('All collections'),
    1 => t('This collection'),
  );
  $form['simple'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'container-inline',
      ),
    ),
  );
  $form['simple']['hidden_filter'] = array(
    '#type' => 'hidden',
    '#value' => $hidden_filter,
  );
  if (!empty($hidden_filter)) {
    $form['simple']['collection_select'] = array(
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => (!empty($hidden_filter)) ? 1 : 0,
    );
  } else {
    $form['simple']['collection_all'] = array(
      '#markup' => '<h2 class="block-title">Search All Collections:</h2>',
    );
  }
  $form['simple']["islandora_simple_search_query"] = array(
    '#size' => '15',
    '#type' => 'textfield',
    '#title' => '',
    // @TODO: should this be the searched value?
    '#default_value' => '',
  );
  $form['simple']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('search'),
  );
  return $form;
}

/**
 * Islandora Solr simple search form submit.
 */
function manidora_simple_search_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $search_string = $form_state['values']['islandora_simple_search_query'];
  // Replace the slash so url doesn't break.
  $search_string = str_replace('/', '~slsh~', $search_string);
  preg_match('/:"(.*?)"/', $form_state['values']['hidden_filter'], $hidden_match);
  $hidden_filter = $hidden_match[1];
  $collection_pid = $hidden_filter;
  if (isset($form_state['values']['collection_select'])) {
    $collection_select = $form_state['values']['collection_select'];
  }

  // Using edismax by default for manidora.
  $query = array('type' => 'edismax');
  if (!empty($hidden_filter) && isset($collection_select) && $collection_select == 1) {
    $query['cp'] = $collection_pid;
  }
  drupal_goto('islandora/search/' . $search_string, array('query' => $query));
}

/**
 * Collect collections under Root Collection PID to display
 */
function __manidora_homepage_tabs_content(){
  /**
   * @todo Ideally the content for the Fedora derived tabs (departments, subjects, format) should be stored in a tab and administered through Drupal.
   *   Unfortunately, due to time-constraints we are hard-coding
   */
   $fedora_object = islandora_object_load('islandora:root');
   
   
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
       "glass lantern slide",
      "photograph",
      "slide",
      "stereo photograph",
      "still image",
    ),
    "Text" => array(
      "text",
      "textual record",
    ),
    "Newspapers" => array("newspaper"),
    "Books" => array(),
    "Moving images" => array("moving image"),
    "Sound recordings" => array(
      "sound recording",
      "sound recording - nonmusical"
    ),
    "Mixed material" => array("mixed material"),
  );
  $departments = array(
    "Archives & Special Collections" => "uofm:archives",
    "Faculty of Architecture" => "",
    "Faculty of Medicine Archives" => "uofm:medArchives",
    "Restorative Dentistry Collection" => "uofm:dentalDAMS",
  //  "Libraries: Newspapers" => "uofm:libraries",
  );
  

   
  
  $output = <<<EOF
    <ul class="home-tabs-container">
    <li class="home-tab active"><a id="home-tabs-subjects" class="home-tab-link" href="javascript:void(0)">Subjects</a></li>
    <li class="home-tab"><a id="home-tabs-collections" class="home-tab-link" href="javascript:void(0)">Collections</a></li>
    <li class="home-tab"><a id="home-tabs-formats" class="home-tab-link" href="javascript:void(0)">Formats</a></li>
    <li class="home-tab"><a id="home-tabs-depts" class="home-tab-link" href="javascript:void(0)">Departments</a></li>
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
      $output .= "$sub<br />\n";
      $count += 1;
    } 
    $output .=<<<EOF
      </div><!-- /#.column_50 -->
    </div><!-- /#home-panel-subjects -->
    <div class="home-panel" id="home-panel-collections">
      <div class="column_50 first">
EOF;
    $ri_query = "select \$pid \$title from <#ri> where \$pid <dc:title> \$title and  \$pid <fedora-model:state> <info:fedora/fedora-system:def/model#Active> and (\$pid <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:collectionCModel> or \$pid <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:newspaperCModel>) order by \$title";
    /**
     * @todo When they load actual objects to Fedora (or when we do), try to restrict collections to uofm ones
     */
    // and \$pid <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> <info:fedora/uofm:top>
      try{
        $objects = $fedora_object->repository->ri->itqlQuery($ri_query, 20);
        $count = 0;
        foreach ($objects as $obj){
          if ($count == 11){
            $output .= "</div><!-- /.column_50 -->\n";
            $output .= "<div class=\"column_50 last\">\n";
          }
          $output .= $obj['title']['value'] . "<br />\n";
          $count += 1;
        }
      } catch (Exception $e){
        error_log("Error in RI query " . $e->getMessage(),0);
        $output .= "Unable to retrieve collections";
      }
      
    $output .=<<<EOF
      </div><!-- /.column_50 -->
    </div><!-- /#home-panel-collections -->
    <div class="home-panel" id="home-panel-formats">
      <div class="column_50 first">
EOF;
    foreach ($formats as $key => $arr){
      $uri = "";
      if (is_array($arr) && count($arr)>0){
        foreach ($arr as $arr_ele){
          $uri .= (!empty($uri)? "%20OR%20":"") . "type_of_resource_mt%3a(".urlencode($arr_ele).")";
        }
  //      $uri = "f[0]=" . $uri;
      }
      if (!empty($uri)){
        $output .= "<a href=\"".url("islandora/search/")."$uri"."\">$key</a><br />\n";
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
        $theURL = url("islandora/object/$uri");
        $output .= "<a href=\"$theURL\">$dept</a><br />\n";
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
    if (count($t_array)>=$num_pics){
      $picked = array_rand($t_array,$num_pics);
      $output = '<div id="manidora-homepage-thumbs">';
      $count = 0;
      foreach ($picked as $pid){
        $output .= '<a href="' . url("islandora/object/" . urlencode($t_array[$pid])) .'" class="manidora-homepage-thumb-link"><img src="' . url("islandora/object/" . urlencode($t_array[$pid]) . "/datastream/TN/view") . '" alt="Object '. $t_array[$pid] .'" class="manidora-homepage-thumb-img" /></a>' . "\n";
        $count += 1;
      }
      $output .= "</div> <!-- /#manidora-homepage-thumbs -->\n";
    }
  }
  return $output;
}