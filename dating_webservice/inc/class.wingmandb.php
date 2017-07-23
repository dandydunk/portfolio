<?php
class WingManDB {
    static public function GetConn() {
        global $config;

        $pdo = new PDO("mysql:host={$config['dbHost']};dbname={$config['dbName']}", $config['dbUser'], $config['dbPassword']);
        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

        return $pdo;
    }
}