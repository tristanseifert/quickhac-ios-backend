<?php
	define("API_LOADED", 1);
	require_once("grade_parse.php");
	
	// Get API method from URL (rewritten)
	$args = explode('/', $_SERVER['DOCUMENT_URI']);  // DOCUMENT_URI = REDIRECT_URL on Apache
	array_shift($args);
	
	$api_method = $args[0];
	
	// Database stuff
	$conn, $db = null;
	
	// Run the appropriate code for the API method
	switch($api_method) {
		/*
		 * Checks if the UUID of the device ("uuid") is already registered in the
		 * database, and if not, attempts to register it with a username, password,
		 * and student ID.
		 */
		case "register": {			
			break;
		}
		
		/*
		 * Associates the push token with the device with a UUID.
		 */
		case "pushRegister": {
			break;
		}
			
		/*
		 * Returns the latest grades that were scraped during a cronjob from the
		 * server.
		 */
		case "fetchLastGrades": {
			break;
		}
			
		/*
		 * Pushes this device to the top of the "refresh queue" that is performed
		 * every 2 minutes.
		 */
		case "requestRefresh": {
			break;
		}
			
		/*
		 * Returns the assignments in a certain class for the specified marking
		 * period.
		 */
		case "fetchAssignments": {
			break;
		}
			
		/*
		 * Updates the device settings, such as refresh rate.
		 */
		case "updateSettings": {
			break;
		}
			
		default: {
			header("HTTP/1.0 400 Invalid Request");
			die(json_encode(array("error" => "Undefined API method")));
			break;
		}
	}
	
	/*
	 * Establishes connection to MongoDB server. Call only if database access is
	 * needed.
	 */
	function connect_mongo() {
		global $conn, $db;
		
		$conn = new Mongo("localhost");
		$db = $conn->qhac_ios;
	}
?>

