<?php
    
require_once("phpQuery.php");
    
/*
 * Takes a HTML document in $html as an input and extracts overall class grades 
 * from it for all marking periods that have data available.
 */
function parse_overall_html($html) {
    // Shut up about the damn warnings pls
    $DOM = new DOMDocument;
    $DOM->preserveWhitespace = false;
    @$DOM->loadHTML($html);
    
    // First table holds grades
    $tables = $DOM->getElementsByTagName("table");
    $table = $tables->item(1);
    
    // Get rows
    $rows = $table->getElementsByTagName("tr");
    
    // We will plop the grades into this array
    $gradesArray = array();
    
    // Loop through all rows
    foreach ($rows as $row) {
        // Get instructor name
        $instructorName = $row->getElementsByTagName("th")->item(0)->textContent;
        
        $courseInfo = array();
        $courseInfo["instructor"] = $instructorName;
        $courseInfo["grades"] = array();
        
        // Grab the instructor's email as well, if we have it
        $instructorEmail = $row->getElementsByTagName("th")->item(0)->firstChild;
        if($instructorEmail->nodeName == "a") {
            $tokens = explode(":", $instructorEmail->attributes->getNamedItem("href")->nodeValue);
            $courseInfo["instructorEmail"] = $tokens[1];
        }
        
        $gradeIndex = 0;
        // I hope the spirits like us and the planets line up right and they won't change their HTML
        foreach($row->getElementsByTagName("td") as $column) {
            if($column->hasAttributes()) { // Teacher name has "align='left'" property
                if($column->attributes->getNamedItem("align")->nodeValue == "left") {
                    $courseTitle = $column->textContent;
                    $courseInfo["title"] = $courseTitle;
                }
            } else { // Interpret everything else as a grade
                $gradeArray = array();
                
                // If the grade is a space, there's no entered grade
                if($column->textContent == " ") {
                    $gradeArray["grade"] = -1;
                } else {
                    // The first "grade" is really the block number
                    if($gradeIndex == 0) {
                        $courseInfo["block"] = intval($column->textContent);                        
                    } else {                    
                        // We need to extract the URL for the link to access MP info
                        $gradeLink = $column->firstChild;
                    
                        if($gradeLink->nodeName == "a") {
                            $gradeArray["href"] = $gradeLink->attributes->getNamedItem("href")->nodeValue;
                        }
                    
                        // Stuff integer grade into array
                        $gradeArray["grade"] = intval($column->textContent);
                    }
                }
                
                $courseInfo["grades"][$gradeIndex] = $gradeArray;
                $gradeIndex++;
            }
        }
        
        // Add this course's info to if it is of use to us
        if($courseInfo["instructor"] != "Teacher") {
            unset($courseInfo["grades"][0]);
            $courseInfo["grades"] = array_values($courseInfo["grades"]);
            $gradesArray[] = $courseInfo;
        }
    }
    
    return $gradesArray;
}
    
// Uncomment this to test
$grades = parse_overall_html(file_get_contents("gradebook.html"));
echo json_encode($grades);
    
?>