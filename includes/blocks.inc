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
    'cache' => DRUPAL_CACHE_PER_USER,
  );
  $blocks['manidora_homepage_thumbnails'] = array(
    'info' => t('Manidora homepage thumbnails'),
    'cache' => DRUPAL_CACHE_GLOBAL,
  );
  $blocks['manidora_whats_new'] = array(
    'info' => t("Manidora what's new"),
    'cache' => DRUPAL_CACHE_PER_USER,
  );
  $blocks['manidora_preview_image'] = array(
    'info' => t('Manidora preview thumbnail'),
    'cache' => DRUPAL_CACHE_PER_USER,
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

    case 'manidora_whats_new':
      $block['subject'] = 'What\'s New';
      $block['content'] = __manidora_whats_new_content();
      break;

    case 'manidora_preview_image':
      $block['subject'] = "Preview";
      $block['content'] = __manidora_preview_image();
      break;

  }
  return $block;
}

/**
 * Creates the homepage tabs content.
 *
 * @todo Ideally the content for the Fedora derived tabs (departments,
 * subjects, format) should be stored in a tab and administered through
 * Drupal. Unfortunately, due to time-constraints we are hard-coding.
 */
function __manidora_homepage_tabs_content() {
  module_load_include('php', 'islandora_solr', 'SolrPhpClient/Apache/Solr/Service');
  module_load_include('inc', 'islandora_solr', 'includes/query_processor');

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
      "solr_field" => "type_of_resource_facet_ms",
      "data" => array(
        "glass lantern slide",
        "photograph",
        "slide",
        "stereo photograph",
        "still image",
      ),
    ),
    "Text" => array(
      "solr_field" => "type_of_resource_facet_ms",
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
      "solr_field" => "type_of_resource_facet_ms",
      // "solr_field" => "dc.type",
      "data" => array(
        "moving image",
      ),
    ),
    "Sound recordings" => array(
      "solr_field" => "type_of_resource_facet_ms",
      "data" => array(
        "sound recording",
        "sound recording - nonmusical",
      ),
    ),
    "Mixed material" => array(
      "solr_field" => "type_of_resource_facet_ms",
      "data" => array(
        "mixed material",
      ),
    ),
  );

  $departments = array(
    // "Faculty of Architecture" => "",
    // "Faculty of Dentistry" => "uofm:dentalDAMS",
    "Faculty of Medicine Archives" => "uofm:medArchives",
    "Libraries: Architecture/Fine Arts Library" => "uofm:archfa",
    "Libraries: Archives & Special Collections" => "uofm:archives",
    "Libraries: Icelandic Collection" => "uofm:icelandic",
    "Libraries: Newspapers" => "uofm:libraries",
    "Restorative Dentistry Collection" => "uofm:restorativedentistry",
    "UM Marketing Communications Office" => "uofm:marketing",
    "UM Office of the Secretary" => "uofm:umsecretary",
  );

  $output = <<<EOF
  <ul class="home-tabs-container">
  <li class="home-tab active"><a title="List Collections Alphabetically" id="home-tabs-collections" class="home-tab-link" href="javascript:void(0)">Collections</a></li>
  <li class="home-tab"><a title="List Collections by Subject" id="home-tabs-subjects" class="home-tab-link" href="javascript:void(0)">Subjects</a></li>
  <li class="home-tab"><a title="Browse by Format type" id="home-tabs-formats" class="home-tab-link" href="javascript:void(0)">Formats</a></li>
  <li class="home-tab"><a title="List Collections by UM Faculty or Department" id="home-tabs-depts" class="home-tab-link" href="javascript:void(0)">Departments</a></li>
  </ul>
  <div class="home-panel" id="home-panel-subjects">
    <div class="column_50 first">
EOF;
  $half = intval(count($subjects) / 2) + (count($subjects) % 2);
  $count = 0;
  foreach ($subjects as $sub) {
    if ($count == $half) {
      $output .= "</div><!-- /.column_50 -->\n";
      $output .= "<div class=\"column_50 last\">\n";
    }
    $output .= l($sub, 'islandora/search/subject_topic_facet_ms:"' . $sub . '"') . "<br />\n";
    $count += 1;
  }
  $output .= <<<EOF
      </div><!-- /#.column_50 -->
    </div><!-- /#home-panel-subjects -->
    <div class="home-panel active" id="home-panel-collections">
EOF;

  $solr_build = new IslandoraSolrQueryProcessor();
  $solr_query = '(RELS_EXT_isMemberOfCollection_uri_mt:uofm* AND RELS_EXT_hasModel_uri_ms:("info:fedora/islandora:collectionCModel" "info:fedora/islandora:newspaperCModel"))';
  $solr_params = array(
    "fl" => "PID,dc.title,fgs_label_s",
    'fq' => '-PID:("uofm:libraries" "uofm:architecture" "uofm:medArchives" "uofm:archives")',
    'rows' => 100000,
    'limit' => 100000,
    'facet' => 'false',
    'hl' => 'false',
  );
  $solr_build->buildQuery($solr_query, $solr_params);
  $solr_build->solrParams = array_merge_recursive($solr_build->solrParams, $solr_params);
  // We don't need faceting or highlighting here.
  $solr_build->solrParams['facet'] = $solr_build->solrParams['hl'] = 'false';

  // Query is executed.
  try {
    $results = $solr_build->executeQuery(FALSE);
  }
  catch (Exception $e) {
    drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $e->getMessage(), 'error');
  }

  if ($results) {
    $get_nested = function($o, $name) {
      if (isset($o[$name])) {
        $tmp = $o[$name];
      }
      else if (isset($o['solr_doc'][$name])) {
        $tmp = $o['solr_doc'][$name];
      }
      if (isset($tmp)) {
        if (is_array($tmp)) {
          return reset($tmp);
        }
        else {
          return $tmp;
        }
      }
      return "";
    };
    $solr_results = json_decode($results->getRawResponse(), TRUE);
    $clean = function($o) use ($get_nested) {
      $title = $get_nested($o, 'dc.title');
      if (empty($title)) {
        $title = $get_nested($o, 'fgs_label_s');
      }
      return array(
        'PID' => $get_nested($o, 'PID'),
        'title' => $title,
      );
    };

    $tmpobjects = array_map($clean, $solr_results['response']['docs']);
    // Solr can't sort by multivalued elements, so sort by title.
    usort($tmpobjects, create_function('$a,$b', 'return strcasecmp($a["title"],$b["title"]);'));
    $objects = array();
    $cl = "";
    /* Move the items so each letter that exists is an array of
    those collections. */
    array_walk($tmpobjects, function($v, $k) use (&$objects, &$cl) {
      if (substr(strtolower(preg_replace('/^the\s?/i', "", $v['title'])), 0, 1) != $cl) {
        $cl = substr(strtolower(preg_replace('/^the\s?/i', "", $v['title'])), 0, 1);
      }
      if (array_key_exists($cl, $objects) && !is_array($objects[$cl])) {
        $objects[$cl] = array();
      }
      $objects[$cl][] = $v;
    });
    $alpha_bar = "<span class=\"coll_alpha_list\">";
    $tmp = "<div class=\"manidora-collection-scroll\">";
    for ($letter_code = 97; $letter_code < (97+26); $letter_code += 1) {
      $letter = chr($letter_code);
      if (isset($objects[$letter])) {
        $alpha_bar .= '<a href="#" onclick="jQuery(\'#coll_' . chr($x) . '\').scroll"> ' . chr($x) . ' </a>|';
        $odd = array();
        $even = array();
        $both = array(&$even, &$odd);
        /* Split items to even and odd array :
        http://stackoverflow.com/a/12405336 */
        array_walk($collections, function($v, $k) use ($both) {
          $both[$k % 2][] = $v;
        });
        $both = array_merge($even, $odd);
        $tmp .= "<div class=\"coll_letter_box\"><a name=\"collection_$letter\" id=\"collection_$letter\" class=\"collection_spacer\"></a>";
        $tmp .= "<div class=\"column_50 first collections\">";
        $count = 0;
        foreach ($both as $c) {
          if ($count == count($even)) {
            $tmp .= "</div><!-- /.column_50 -->\n";
            $tmp .= "<div class=\"column_50 last\">\n";
          }
          $tmp .= l($c['title'], 'islandora/object/' . $c['PID']) . "<br />\n";
          $count += 1;
        }
        $tmp .= "</div> <!-- /.column_50 -->\n";
        $tmp .= "</div> <!-- /.coll_letter_box -->\n";
      }
      else {
        $alpha_bar .= ' ' . chr($x) . " |";
      }
    }
    $tmp .= "</div> <!-- /.manidora-collection-scroll-box-->\n";
    $alpha_bar = substr($alpha_bar, 0, -2) . "</span>";

    $output .= $alpha_bar + $tmp;
  }

  $output .= <<<EOF
    </div><!-- /#home-panel-collections -->
    <div class="home-panel" id="home-panel-formats">
      <div class="column_50 first">
EOF;
  foreach ($formats as $key => $format) {
    $uri = "";
    if (array_key_exists('data', $format) && is_array($format['data']) && count($format['data']) > 0) {
      foreach ($format['data'] as $ele) {
        $uri .= (!empty($uri) ? " OR " : "") . '"' . $ele . '"';
      }
      $uri = $format['solr_field'] . ':(' . $uri . ')';
      if (!empty($uri)) {
        $uri = l($key, "islandora/search/$uri", array('html' => TRUE));
      }
    }
    elseif (array_key_exists('url', $format)) {
      $uri = l($key, $format['url']);
    }
    if (!empty($uri)) {
      $output .= "$uri<br />\n";
    }
    else {
      $output .= "$key<br />\n";
    }
  }
  $output .= <<<EOF
      </div><!-- /.column_50 -->
    </div><!-- /#home-panel-formats -->
    <div class="home-panel" id="home-panel-depts">
      <div class="column_50 first">
EOF;
  foreach ($departments as $dept => $uri) {
    if (!empty($uri)) {
      $output .= l($dept, "islandora/object/$uri") . "<br />\n";
    }
    else {
      $output .= "$dept<br />\n";
    }
  }
  $output .= <<<EOF
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
      jQuery('a[href^="#"]', '#home-panel-collections').click(function(e) {
        e.preventDefault();
        var href = jQuery(this).attr('href');
        var target = jQuery(href, '#home-panel-collections');
        target.scrollParent().scrollTop(target.scrollParent().scrollTop() + target.position().top);
      });
    });
    //-->
    </script>
EOF;

  return $output;

}

/**
 * Display 16 thumbnails on the homepage from those selected.
 */
function __manidora_homepage_thumbnail_content() {
  module_load_include('php', 'islandora_solr', 'SolrPhpClient/Apache/Solr/Service');
  // How many thumbnails are we displaying?
  $num_pics = 16;
  // Figure out when to break.
  $num_rows = 2;
  // Get the pids from the variable.
  $thumbs = variable_get('manidora_thumbnail_pids');
  $output = "";
  if (!is_null($thumbs)) {
    // If its null we display nothing.
    $t_array = explode('~', urldecode($thumbs));
    /* This is temporary for debugging, we extend the array by
    duplicating it to see the full selection. */
    if (count($t_array) < $num_pics) {
      $dup = intval($num_pics / count($t_array)) + 1;
      $new_arr = array();
      for ($foo = 0; $foo < $dup; $foo += 1) {
        $new_arr = array_merge($new_arr, $t_array);
      }
      $t_array = $new_arr;
    }
    /* End temporary extension. */
    // We want to have the image title as mouseover in the block.
    // Search for all the possible PIDs.
    $solr_query = '+PID:("' . implode('" "', $t_array) . '")';
    // Get the PID and dc.title.
    $solr_params = array('fl' => 'PID,title_sort_ms');
    $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
    // $path_parts = parse_url('digitalcollections.lib.umanitoba.ca:8080/solr');
    $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');

    $solr->setCreateDocuments(0);
    $results = $solr->search($solr_query, 0, 100, $solr_params);
    if ($results) {
      $solr_results = json_decode($results->getRawResponse(), TRUE);
      $data = array();
      foreach ($solr_results["response"]["docs"] as $key => $doc) {
        $data[$doc['PID']] = (array_key_exists('title_sort_ms', $doc) ? $doc['title_sort_ms'][0] : "");
      }
    }
    if (count($t_array) >= $num_pics) {
      $picked = array_rand($t_array, $num_pics);
      $output = '<div id="manidora-homepage-thumbs">';
      foreach ($picked as $pid) {
        $title = '';
        // If $data is an array and has a key of the current PID,
        // then we know the title.
        if (isset($data) && is_array($data) && array_key_exists($t_array[$pid], $data)) {
          $title = 'title="' . check_plain($data[$t_array[$pid]]) . '" ';
        }
        $output .= '<a ' . $title . 'href="' . url("islandora/object/" . $t_array[$pid]) . '" class="manidora-homepage-thumb-link"><img src="' . url("islandora/object/" . $t_array[$pid] . "/datastream/TN/view") . '" alt="Object ' . check_plain($t_array[$pid]) . '" class="manidora-homepage-thumb-img" /></a>' . "\n";
      }
      $output .= "</div> <!-- /#manidora-homepage-thumbs -->\n";
    }
  }
  return $output;
}

/**
 * Generate 4 most recently added items from Solr timestamp.
 */
function __manidora_whats_new_content() {
  module_load_include('php', 'islandora_solr', 'SolrPhpClient/Apache/Solr/Service');
  module_load_include('inc', 'islandora_solr', 'includes/query_processor');
  module_load_include('inc', 'islandora_restricted', 'includes/utilities');
  $solr_build = new IslandoraSolrQueryProcessor();

  $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');

  $query = 'fgs_label_s:* AND fgs_createdDate_dt:* AND fedora_datastreams_ms:TN';
  $params = array(
    'fl' => 'PID,fgs_label_s,fgs_createdDate_dt,fedora_datastreams_ms,RELS_EXT_canSeeRestricted_uri_ms,RELS_EXT_restrictedState_literal_ms',
    'sort' => 'fgs_createdDate_dt desc',
    'fq' => array(
      '-RELS_EXT_hasModel_uri_ms:("info:fedora/islandora:newspaperPageCModel" "info:fedora/islandora:pageCModel")',
    ),
    'facet' => 'false',
    'hl' => 'false',
    'rows' => 4,
    'limit' => 4,
  );
  $solr->setCreateDocuments(0);
  $solr_build->buildQuery($query, $params);

  $solr_build->solrParams = array_merge_recursive($solr_build->solrParams, $params);
  $solr_build->solrParams['facet'] = $solr_build->solrParams['hl'] = 'false';
  
  for ($foo = 0; $foo < count($solr_build->solrParams['fq']); $foo += 1) {
    if ($solr_build->solrParams['fq'][$foo] == '-RELS_EXT_hasModel_uri_ms:"info:fedora/islandora:newspaperIssueCModel"') {
      // We need to stop stripping out newspaperIssues from here too.
      unset($solr_build->solrParams['fq'][$foo]);
    }
  }
  try {
    $solr_query = ($solr_build->internalSolrQuery) ? $solr_build->internalSolrQuery : $solr_build->solrQuery;
    $method = 'GET';
    $results = $solr->search($solr_query, $solr_build->solrStart, $solr_build->solrParams['rows'], $solr_build->solrParams, $method);
    $solr_results = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $e->getMessage(), 'error');
  }
  $output = "<div class=\"manidora-whats-new-content\">";
  $map = function($o) {
    return array(
      'pid' => $o['PID'],
      'title' => $o['fgs_label_s'],
      'date' => $o['fgs_createdDate_dt'],
    );
  };
  if ($solr_results) {
    $results = array_map($map, $solr_results['response']['docs']);
    foreach ($results as $result) {
      $output .= theme('manidora_whats_new_item', array('item' => $result));
    }
  }
  $output .= "</div>";
  return $output;
}

/**
 * Theme the manidora-whats-new list items.
 */
function manidora_preprocess_manidora_whats_new_item(&$variables) {
  global $user;
  module_load_include('inc', 'islandora_restricted', 'includes/utilities');

  $new_item = array();
  $item = $variables['item'];
  $object = islandora_object_load($item['pid']);
  if ($object) {
    $variables['pid'] = $item['pid'];
    $tstamp = DateTime::createFromFormat('Y-m-d\TH:i:s.uZ', $item['date']);
    $variables['timestamp'] = $tstamp;
    $title = $item['title'];
    if (!islandora_restricted_user_has_access($object, $user)) {
      $title .= " - Restricted";
      $image_path = drupal_get_path('module', 'islandora_restricted') . '/images/access_denied.png';
    }
    elseif (isset($object['TN'])) {
      $image_path = 'islandora/object/' . $item['pid'] . '/datastream/TN/view';
    }
    else {
      $image_path = drupal_get_path('module', 'islandora', '/images/folder.png');
    }
    $stitle = (strlen($title) > 30 ? substr($title, 0, strpos(wordwrap($title, 25, '~'), '~')) . '...' : $title);
    $variables['title'] = $title;
    $variables['image'] = t('<a href="@link"><img src="@url" alt="@title" class="featured-thumb"/></a>', array(
      '@url' => url($image_path),
      '@title' => $stitle,
      '@link' => url('islandora/object/' . $item['pid']),
    ));
    $variables['link'] = l($stitle, 'islandora/object/' . $item['pid']);
  }
  unset($variables['item']);
}

/**
 * Check to see if What's New can show this.
 *
 * $doc : Mixed array of a Solr Document
 * $roles : Users roles modified to structured used by islandora_restricted
 *
 * return int : 0 => public, 1 => restricted, 2 => hidden
 */
function __manidora_check_restricted_permissions($doc, $roles) {
  if (array_key_exists('RELS_EXT_restrictedState_literal_ms', $doc)) {
    if (count(array_intersect($roles, $doc['RELS_EXT_canSeeRestricted_uri_ms'])) > 0) {
      // If the person has the role, then it is public to them
      // regardless if it is hidden or restricted.
      return 0;
    }
    elseif (in_array('hidden', $doc['RELS_EXT_restrictedState_literal_ms'])) {
      return 2;
    }
    elseif (in_array('restricted', $doc['RELS_EXT_restrictedState_literal_ms'])) {
      return 1;
    }
  }
  return 0;
}

/**
 * Display the thumnbnail when editing the metadata form.
 */
function __manidora_preview_image() {
  global $user;
  module_load_include('inc', 'islandora_restricted', 'includes/utilities');
  $path = current_path();
  $path_parts = explode('/', $path);
  $view = FALSE;
  $output = "";
  foreach ($path_parts as $p) {
    if (strpos($p, ':') > 0) {
      $pid = $p;
      $obj = islandora_object_load($pid);
      if (!is_null($obj) && FALSE !== $obj) {
        if ($obj->getDatastream('TN') !== FALSE) {
          if (islandora_restricted_retrieve_restrictions($obj, $user)) {
            $view = TRUE;
            break;
          }
        }
      }
      $obj = NULL;
    }
  }
  if ($view) {
    drupal_add_library('system', 'ui.draggable');
    drupal_add_js(drupal_get_path('module', 'manidora') . '/js/preview.js');
    $title = t('Preview');
    $url = url('islandora/object/' . $pid . '/datastream/TN/view');
    $output = <<<EOF
<div class="manidora-preview-image" id="image-$pid">
  <h1 class="block-title">$title</h1>
  <img src="$url" />
</div>
EOF;
  }
  return $output;
}
