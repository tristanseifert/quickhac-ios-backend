<?php
	define("API_LOADED", 1);
	require_once("grade_parse.php");
	require_once("secret.php"); // See README.md for info about this

	// Send JSON content-type header
	header("Content-Type: application/json");

	// Get API method from URL (rewritten)
	$args = explode('/', $_SERVER['DOCUMENT_URI']);  // DOCUMENT_URI = REDIRECT_URL on Apache
	array_shift($args);
	
	$api_method = $args[count($args)-1];
	
	// Database stuff
	$conn = null;
	$db = null;
	
	// Run the appropriate code for the API method
	switch($api_method) {
		/*
		 * Checks if the UUID of the device ("uuid") is already registered in the
		 * database, and if not, attempts to register it with a username, password,
		 * and student ID.
		 */
		case "register": {
			$deviceUUID = $_POST["uuid"];
			$hacUser = $_POST["username"];
			$hacPass = $_POST["password"];
			$hacID = $_POST["sid"];
			
			connect_mongo();

			$collection = $db->devices;
			$cursor = $collection->find(array("uuid" => $deviceUUID));

			if($cursor->count() > 0) {
				$deviceInfo = $cursor->getNext();
				die(json_encode(array("status" => 200, "message" => "Device already registered.", "id" => (string) $deviceInfo["_id"])));
			} else {
				$encrypted_pass = encrypt_password($hacPass);

				$deviceArray = array();
				$deviceArray["user"] = $hacUser;
				$deviceArray["password"] = $encrypted_pass;
				$deviceArray["uuid"] = $deviceUUID;
				$deviceArray["sid"] = $hacID;

				// Insert device array into database
				$collection->insert($deviceArray);
			}

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

