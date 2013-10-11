<?php
/*
 * Takes a HTML document in $html as an input and extracts overall class grades 
 * from it for all marking periods that have data available.
 */
function parse_overall_html($html) {
	// Shut up about the damn warnings pls
	$DOM = new DOMDocument;
	$DOM->preserveWhitespace = false;
	@$DOM->loadHTML($html);
	
	// Find all tables
	$tables = $DOM->getElementsByTagName("table");
	$table = null;

	foreach($tables as $iterateTable) {
		if($iterateTable->hasAttributes()) {
			if($iterateTable->attributes->getNamedItem("class")->nodeValue == "DataTable") {
				$table = $iterateTable;
				break;
			}
		}
	}
	
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
		
		// I hope the spirits like us and the planets line up right and they won't change their HTML
		$gradeIndex = 0;
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


/*
 * Takes a HTML document in $html as an input and extracts all of the assignments
 * and their associated categories.
 */
function parse_grade_array($html) {
	// Shut up about the damn warnings pls
	$DOM = new DOMDocument;
	$DOM->preserveWhitespace = false;
	@$DOM->loadHTML($html);
    
    // Hold categories
	$categoryArray = array();
    
    // Loop through all SPAN elements with the class "CategoryName"
	$spans = $DOM->getElementsByTagName("span");
	foreach($spans as $span) {
		if($span->hasAttributes()) {
			if($span->attributes->getNamedItem("class")->nodeValue == "CategoryName") {
				$titlePattern = '/([A-Za-z]+)\W+(\d+%)/';
				preg_match($titlePattern, $span->nodeValue, $matches);
				
				$category = array();
				$category["title"] = $matches[1];
				$category["weight"] = intval($matches[2]) / 100;
				$categoryArray[] = $category;
			}
		}
	}
		
	// Find all tables
	$tables = $DOM->getElementsByTagName("table");
	$table = null;
    
    // The first table contains the overall class grades, so use a counter to skip it
    $counter = 0;
	$categoryIndex = 0;
    
	foreach($tables as $table) {
		if($table->hasAttributes()) {
			if($table->attributes->getNamedItem("class")->nodeValue == "DataTable" && $counter > 1) {                
                // Get rows
                $rows = $table->getElementsByTagName("tr");
                
                // Loop through all rows
                foreach ($rows as $row) {
					$class = $row->attributes->getNamedItem("class")->nodeValue;
					
					// Ensure we only parse the rows with relevant data
					if($class == "DataRow" || $class == "DataRowAlt") {
						// Get the columns contained within the row
						if($row->hasChildNodes()) {
							// Array to hold assignment info
							$assignment = array();
							
							$columns = $row->getElementsByTagName("td");
							
							// Iterate through columns.
							foreach($columns as $column) {
								$columnClass = $column->attributes->getNamedItem("class")->nodeValue;
								
								// Populate the assignment array
								switch($columnClass) {
									case "AssignmentName": {
										$assignment["title"] = stripslashes($column->nodeValue);
										break;
									}
									case "DateAssigned": {
										$assignment["assigned"] = $column->nodeValue;
										break;
									}
									case "DateDue": {
										$assignment["due"] = $column->nodeValue;
										break;
									}
									case "AssignmentGrade": {
										$assignment["grade"] = intval($column->nodeValue);
										break;
									}
									case "AssignmentNote": {
										$assignment["note"] = stripslashes($column->nodeValue);
										break;
									}
										
									default:
										break;
								}
							}
							
							if(strlen($assignment["note"]) == 0) {
								unset($assignment["note"]);
							}
						
							$categoryArray[$categoryIndex]["assignments"][] = $assignment;
						}
					}
                }
			
				$categoryArray[$categoryIndex] = $categoryArray[$categoryIndex];
				
				$categoryIndex++;
            }
            
            $counter++;
		}
	}
	
	return $categoryArray;
}
	
// Uncomment this to test
/*$grades = parse_overall_html(file_get_contents("gradebook.html"));
echo json_encode($grades); */

$grades = parse_grade_array(file_get_contents("gradebook_class.html"));
echo var_dump($grades);
?>

