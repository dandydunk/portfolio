<?php
require 'inc/config.php';
$users = WingManUser::GetAll();

function A() {
	if(!isset($_REQUEST['userId'])) {
		return;
	}
	
	if(!isset($_REQUEST['latitude']) || !strlen(trim($_REQUEST['latitude']))) {
		return;
	}
	
	if(!isset($_REQUEST['longitude']) || !strlen(trim($_REQUEST['longitude']))) {
		return;
	}
	var_dump($_REQUEST);
	$user = WingManUser::GetById($_REQUEST['userId']);
	$user->CheckIn($_REQUEST);
	
	header("Location: checkInUsers.php");
	exit;
	
}
A();
?>
<!doctype html>
<html>
<head>
	<title>WingMan Admin</title>
</head>

<body>

<h1>WingMan Admin</h1>

<form action="checkInUsers.php" method="post">
	<div>
		<label>users</label>
		<select name="userId">
		<?php
		foreach($users as $user) {
			echo("<option value='{$user['userId']}'>{$user['firstName']} {$user['lastName']}</option>");
		}
		?>
		</select>
	</div>

	<div>
		<label>latitude</label>
		<input type="text" name="latitude">
	</div>
	<div>
		<label>longitude</label>
		<input type="text" name="longitude">
	</div>
	<div>
		<input type="submit">
	</div>
	<?php

	?>
</form>

</body>
</html>