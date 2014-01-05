<?php
header("Content-Type: application/json");

$uri = $_SERVER["REQUEST_URI"];
$uriComponents = explode("/", $uri);
$api_method = $uriComponents[2];

$request_params = $_POST;

// Connect to the DB
$m = new MongoClient();
$db = $m->selectDB("qhac");

// Retrieve the device record.
$uuid = $request_params["uuid"];
if(!$uuid) {
	request_incomplete("uuid");
}

$account = $db->devices->findOne(array("uuid" => $uuid));

// API methods are defined here.
if($api_method == "register") {
	$push_token = $request_params["token"];

	if(!$push_token) {
		request_incomplete("token");
	}

	// Create it if it doesn't exist
	if(!$account) {
		$account = array("last_update" => new MongoDate(), "uuid" => $uuid, 
						 "push_tokens" => array());
		$db->devices->insert($account);
	}

	// If token is not registered, register it.
	if(!in_array($push_token, $account["push_tokens"])) {
		$account["push_tokens"][] = $push_token;
	}

	// Update "last connected" data
	$account["last_update"] = new MongoDate();

	save_account();
} else {
	header("HTTP/1.1 404 Not Found");
	die(json_encode(array("err" => "API method $api_method does not exist.")));
}

/**
 * Saves the account data back to the database.
 */
function save_account() {
	global $account, $db;

	$db->devices->update(array("_id" => $account["_id"]), $account);
}

/**
 * Helper method that returns an error code and prints an error message.
 */
function request_incomplete($param = null) {
	header("HTTP/1.1 406 Not Acceptable");

	if($param) {
		$e = array("err" => "The request was incomplete. The $param field is missing.");
	} else {
		$e = array("err" => "The request was incomplete.");
	}

	die(json_encode($e));
}

?>
