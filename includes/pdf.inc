<?php
/**
 * @file
 * PDF specific functions
 */

/**
 * Menu callback downloads the given clip.
 */
function manidora_download_clip_pdf(AbstractObject $object) {
  if (isset($_GET['clip'])) {
    $url = $_GET['clip'];
    if (!preg_match('/^https?:/', $url)) {
      // Only add server name if it isn't there already.
      $is_https = isset($_SERVER['HTTPS']) && strtolower($_SERVER['HTTPS']) == 'on';
      $http_protocol = $is_https ? 'https' : 'http';
      $url = $http_protocol . '://' . $_SERVER['HTTP_HOST'] . $url;
    }
    manidora_printable_pdf($object->id, $url);
  }
  exit();
}




/**
 * Constructs a printable PDF from the JPG datastream with header content.
 *
 * @param string $pid
 *   A PID representing a Fedora object.
 *
 * @return string
 *   Download link containing our created PDF.
 */
function manidora_printable_pdf($pid, $url = NULL) {
  @include_once 'sites/all/libraries/tcpdf/config/lang/eng.php';
  @include_once 'sites/all/libraries/tcpdf/tcpdf.php';
  require_once dirname(__FILE__) . '/utilities.inc';

  // Extend the TCPDF class to create custom Header and Footer.
  class MYPDF extends TCPDF {

    /**
     * Constructor.
     */
    public function __construct($pid, $orientation = 'P', $unit = 'mm', $format = 'A4', $unicode = TRUE, $encoding = 'UTF-8', $diskcache = FALSE, $pdfa = FALSE) {
      $this->pid = $pid;
      parent::__construct($orientation, $unit, $format, $unicode, $encoding, $diskcache);
    }

    /**
     * Page header.
     */
    public function Header() {
      $string = manidora_retrieve_header_text($this->pid);
      if ($string) {
        $this->Cell(0, 15, $string, 0, FALSE, 'C', 0, '', 0, FALSE, 'M', 'M');
      }
    }
  }
  // Create new PDF document.
  $pdf = new MYPDF($pid, PDF_PAGE_ORIENTATION, PDF_UNIT, PDF_PAGE_FORMAT, TRUE, 'UTF-8', FALSE);

  $header_text = manidora_retrieve_header_text($pid);
  if ($header_text) {
    // Parse a bit for use below.
    $header_parts = explode(',', $header_text);
    $title = $header_parts[0];
  }
  else {
    $title = $pid;
    $header_text = $pid;
  }
  // Set document information.
  $pdf->SetTitle($title);
  $pdf->SetSubject($title);

  // Set default monospaced font.
  $pdf->SetDefaultMonospacedFont(PDF_FONT_MONOSPACED);

  // Set margins.
  $pdf->SetMargins(PDF_MARGIN_LEFT, PDF_MARGIN_TOP, PDF_MARGIN_RIGHT);
  $pdf->SetHeaderMargin(PDF_MARGIN_HEADER);

  // Set auto page breaks.
  $pdf->SetAutoPageBreak(TRUE, PDF_MARGIN_BOTTOM);

  // Set image scale factor.
  $pdf->setImageScale(PDF_IMAGE_SCALE_RATIO);

  // Set some language-dependent strings.
  $pdf->setLanguageArray($l);

  // Set font.
  $pdf->SetFont('times', 'BI', 12);

  // If no url was provided (e.g. we're not coming from the clipper),
  // check to see if there's a high quality jpeg derivative.  If it does not
  // exist, default to the standard JPG datastream.
  if (!isset($url)) {
    $obj = islandora_object_load($pid);
    $dsid = isset($obj['HQ_JPG']) ? 'HQ_JPG' : 'JPG';
    $url = variable_get('islandora_base_url', 'http://localhost:8080/fedora') . '/objects/' . $pid . '/datastreams/' . $dsid . '/content';
  }

  // Add a page.
  $pdf->AddPage();
  // Add the image.
  // Gotta specify almost ALL the parameters to get to the 'fitonpage'
  // parameter.  This should fix weird clipping issues between the actual image
  // source and what gets embedded into the PDF.
  $pdf->Image($url, '0', '15', 0, 0, '', '', '', 2, 300, '', FALSE, FALSE, 0, FALSE, FALSE, TRUE);

  // Close and output PDF document.
  $pdf->Output($header_text . '.pdf', 'D');
}
