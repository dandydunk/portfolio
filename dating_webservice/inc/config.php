<?php
$config['dbName'] = 'wingman';
$config['dbHost'] = 'localhost';
$config['dbUser'] = 'root';
$config['dbPassword'] = '';

define('FACEBOOK_APP_ID', '1786091621707514');
define('GOOGLE_API_KEY', 'AIzaSyBUvhcOdRbYVeIpU50ovziCorEjAhnY8IY');
define('PATH_TO_UPLOAD', 'C:\\wamp32\\www\\wingman\\uploads\\');
if(!is_dir(PATH_TO_UPLOAD)) {
    mkdir(PATH_TO_UPLOAD);
}

spl_autoload_register(function ($class) {
	if(!file_exists('inc/class.' . $class . '.php')) {
		return;
	}
    include 'inc/class.' . $class . '.php';
});