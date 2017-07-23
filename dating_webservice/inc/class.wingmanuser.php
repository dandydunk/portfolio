<?php
class WingManUser {
    public $id = 0;
    public $email = '';
    public $password = '';
    public $sessionId = '';
    public $firstName = '';
    public $lastName = '';
	public $userName = '';
    public $birthday = '';
	public $isRobot = 0;
    
    static public function GetAll() {
        $query = "SELECT u.id AS userId, up.firstName, up.lastName "
                . "FROM users AS u "
                . "INNER JOIN userprofiles AS up ON up.userId=u.id";

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->execute();
        
        $results = array();
        while($row = $smt->fetch(PDO::FETCH_ASSOC)) {
            $row['profilePic'] = base64_encode(file_get_contents('ext/noImg.gif'));
            $results[] = $row;
        }
        
        return $results;
    }
	
	public function GetLocation() {
		$query = 'SELECT *
					FROM usercheckins AS uc
					WHERE uc.userId=:userId
				ORDER BY id DESC';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
		
		$smt->bindValue(':userId', $this->id);
		
        $smt->execute();
		
		$row = $smt->fetch();
		
		return $row;
	}
	
	public function GetByRadius_() {
		$sessionUserLocation = $this->GetCurrentLocation();
		if(!$sessionUserLocation) {
			return array();
		}
		
        $query = 'SELECT u.id AS userId, up.firstName, up.lastName, uc.milesFromCentralPoint, 
		ABS(uc.milesFromCentralPoint - :x) AS distanceFromSessionUser
				FROM usercheckins AS uc
				INNER JOIN users AS u ON uc.id = u.currentCheckInId
				INNER JOIN userprofiles AS up ON up.userId=u.id
				WHERE u.id != :sessionUserId
				';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
		$smt->bindValue(':x', $sessionUserLocation['milesFromCentralPoint']);
		$smt->bindValue(':sessionUserId', $this->id);
		
        $smt->execute();
        
        $results = array();
        while($row = $smt->fetch(PDO::FETCH_ASSOC)) {
            $row['profilePic'] = base64_encode(file_get_contents('ext/noImg.gif'));
			$row['distanceFromSessionUser'] = round($row['distanceFromSessionUser']);
            $results[] = $row;
        }
        
        return $results;
    }
	
	public function GetByRadius() {
		$sessionUserLocation = $this->GetLocation();
		if(!$sessionUserLocation) {
			return array();
		}
		
        $query = 'SELECT u.id AS userId, uc.milesFromCentralPoint, 
		ABS(uc.milesFromCentralPoint - :x) AS distanceFromSessionUser
				FROM usercheckins AS uc
				RIGHT JOIN users AS u ON uc.id = u.currentCheckInId
				WHERE u.id != :sessionUserId AND (u.isRobot = 1 OR u.currentCheckInId IS NOT NULL)
				';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
		$smt->bindValue(':x', $sessionUserLocation['milesFromCentralPoint']);
		$smt->bindValue(':sessionUserId', $this->id);
		
        $smt->execute();
        
        $results = array();
        while($row = $smt->fetch(PDO::FETCH_ASSOC)) {
			$row['distanceFromSessionUser'] = round($row['distanceFromSessionUser']);
            $results[] = $row;
        }
        
        return $results;
    }
	
	static public function GetByUserName($id) {
        $query = 'SELECT id  FROM userprofiles WHERE userName=:id LIMIT 1';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $id);
        $smt->execute();

        $result = $smt->fetch();
        if(!$result) {
           return null;
        }

        $wm = new WingManUser();
        $wm->id = $result['id'];
		$wm->isRobot = $result['isRobot'];
        
        return $wm;
    }
    
    static public function GetById($id) {
        $query = 'SELECT id, isRobot FROM users WHERE id=:id LIMIT 1';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $id, PDO::PARAM_INT);
        $smt->execute();

        $result = $smt->fetch();
        if(!$result) {
           return null;
        }

        $wm = new WingManUser();
        $wm->id = $result['id'];
		$wm->isRobot = $result['isRobot'];
        
        return $wm;
    }
    
    static public function GetByEmail($email) {
        $query = 'SELECT id FROM users WHERE email=:email LIMIT 1';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':email', $email, PDO::PARAM_STR);
        $smt->execute();

        $result = $smt->fetch();
        if(!$result) {
                return null;
        }

        $wm = new WingManUser();
        $wm->id = $result['id'];
        $wm->email = $email;

        return $wm;
    }
    
    static public function GetBySessionId($sessionId) {
        $query = 'SELECT id FROM users WHERE sessionId=:sessionId LIMIT 1';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':sessionId', $sessionId, PDO::PARAM_STR);
        $smt->execute();

        $result = $smt->fetch();
        if(!$result) {
                return null;
        }

        $wm = new WingManUser();
        $wm->id = $result['id'];
        $wm->sessionId = $sessionId;

        return $wm;
    }
    
	public function CheckIn($coords) {
		$location = GoogleAPI::GPSToAddress($coords);
		$milesFromCentralPoint = WingManUtils::Distance($coords['latitude'], $coords['longitude'],
														36.244273, -105.117188, 'M');
		
		$query = 'INSERT INTO usercheckins(milesFromCentralPoint, timeCreated, userId, latitude, longitude, 
										city, state, postalCode, address, country, street, place) 
										VALUES(:milesFromCentralPoint, NOW(), :userId, :latitude, :longitude, :city, :state,
										:postalCode, :address, :country, :street, :placeName)';

        $pdo = WingManDB::GetConn();
		$pdo->beginTransaction();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':milesFromCentralPoint', $milesFromCentralPoint);
		$smt->bindValue(':userId', $this->id, PDO::PARAM_INT);
		$smt->bindValue(':latitude', $coords['latitude']);
		$smt->bindValue(':longitude', $coords['longitude']);
		$smt->bindValue(':city', $location['city']);
		$smt->bindValue(':state', $location['state']);
		$smt->bindValue(':postalCode', $location['postalCode']);
		$smt->bindValue(':address', $location['address']);
		$smt->bindValue(':country', $location['country']);
		$smt->bindValue(':street', $location['street']);
		$smt->bindValue(':placeName', $coords['placeName']);
        $smt->execute();
		
		$checkInId = $pdo->lastInsertId();
		$query = 'UPDATE users SET currentCheckInId=:checkInId WHERE id=:userId';
		$smt = $pdo->prepare($query);
        $smt->bindValue(':checkInId', $checkInId, PDO::PARAM_INT);
		$smt->bindValue(':userId', $this->id, PDO::PARAM_INT);
		$smt->execute();
		$pdo->commit();
	}
	
    public function FillProfile() {
        $query = 'SELECT userName, birthday FROM userprofiles WHERE userId=:id LIMIT 1';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $this->id, PDO::PARAM_STR);
        $smt->execute();

        $result = $smt->fetch();
        if($result) {
            $this->userName = $result['userName'];
            $this->birthday = $result['birthday'];
        }
    }
    
    public function CreateProfile() {
        $pdo = WingManDB::GetConn();
        $query = 'INSERT INTO userprofiles(userId) VALUES(:id)';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $this->id, PDO::PARAM_INT);
        
        $smt->execute();
    }
    
    public function CreateProfile2($f, $l) {
        $pdo = WingManDB::GetConn();
        $query = 'INSERT INTO userprofiles(userId, firstName, lastName) VALUES(:id, :f, :l)';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $this->id, PDO::PARAM_INT);
        $smt->bindValue(':f', $f, PDO::PARAM_STR);
        $smt->bindValue(':l', $l, PDO::PARAM_STR);
        
        $smt->execute();
    }
    
    public function UpdateProfileValue($colName, $value) {
        $pdo = WingManDB::GetConn();
        $query = 'UPDATE userprofiles SET '.$colName.' = :value WHERE userId=:id LIMIT 1';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $this->id, PDO::PARAM_INT);
        $smt->bindValue(':value', $value);
        
        $smt->execute();
    }
    
    public function UpdateSessionId() {
        $pdo = WingManDB::GetConn();
        $query = 'UPDATE users SET sessionId = :sessionId WHERE id=:id LIMIT 1';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $this->id, PDO::PARAM_INT);
        $smt->bindValue(':sessionId', $this->sessionId);
        
        $smt->execute();
    }
    
    public function Validate() {
        $query = 'SELECT id FROM users WHERE email=:email AND password=PASSWORD(:password) LIMIT 1';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':email', $this->email, PDO::PARAM_STR);
        $smt->bindValue(':password', $this->password, PDO::PARAM_STR);
        $smt->execute();

        $result = $smt->fetch();
        if($result) {
            $this->id = $result['id'];
        }
        return $result ? true:false;
    }
    
    public function UpdateLastActivity() {
        $pdo = WingManDB::GetConn();
        $query = 'UPDATE users SET lastActivity = NOW() WHERE id=:id LIMIT 1';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':id', $this->id, PDO::PARAM_INT);
        
        $smt->execute();
    }
    
    public function CountPhotos() {
        $query = 'SELECT COUNT(id) AS countPhotos FROM userPhotos WHERE userId=:userId';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':userId', $this->id, PDO::PARAM_INT);
        $smt->execute();

        $result = $smt->fetch();
        return $result['countPhotos'];
    }
	
	public function GetProfilePhotoBytes() {
        $query = 'SELECT path, id, isMain FROM userPhotos WHERE userId=:userId AND isMain=1 LIMIT 1';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':userId', $this->id, PDO::PARAM_INT);
        $smt->execute();
        
		$row = $smt->fetch(PDO::FETCH_ASSOC);
		$result = array();
        if($row && is_file($row['path'])) {
           return base64_encode(file_get_contents($row['path']));
        } 
        
        return null;
    }
    
    public function GetPhotos() {
        $query = 'SELECT path, id, isMain FROM userPhotos WHERE userId=:userId';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':userId', $this->id, PDO::PARAM_INT);
        $smt->execute();
        
        $results = array();
        while($row = $smt->fetch(PDO::FETCH_ASSOC)) {
            if(!is_file($row['path'])) continue;
            $row['picBytes'] = base64_encode(file_get_contents($row['path']));
            $results[] = $row;
        } 
        
        return $results;
    }
    
    public function DeletePhoto($id) {
        $query = 'DELETE FROM userPhotos WHERE id=:id AND userId=:userId';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':userId', $this->id, PDO::PARAM_INT);
        $smt->bindValue(':id', $id, PDO::PARAM_INT);
        $smt->execute();
    }
    
    public function SavePhoto($photoEncodedBytes) {
        $decodedBytes = base64_decode($photoEncodedBytes);
        $fileName = PATH_TO_UPLOAD.'img_'.$this->id.'_'.$this->CountPhotos().'.jpg';
        file_put_contents($fileName, $decodedBytes);
        
        $query = 'INSERT INTO userPhotos(path, isMain, userId) VALUES(:path, :isMain, :userId)';

        $pdo = WingManDB::GetConn();
        $smt = $pdo->prepare($query);
        $smt->bindValue(':userId', $this->id, PDO::PARAM_INT);
        $smt->bindValue(':isMain', 0, PDO::PARAM_INT);
        $smt->bindValue(':path', $fileName, PDO::PARAM_STR);
        $smt->execute();
    }
    
    public function Save() {
        $user = WingManUser::GetByEmail($this->email);
        if($user && $user->id != $this->id) {
                throw new Exception('the email exists for another user.');
        }

        $pdo = WingManDB::GetConn();

        if($this->id) {
                $query = 'UPDATE users SET email=:email WHERE id=:id';
                $smt = $pdo->prepare($query);
                $smt->bindValue(':id', $this->id, PDO::PARAM_INT);
                $smt->bindValue(':email', $this->email);
        }

        else {
                $query = 'INSERT INTO users(email, password) VALUES(:email, PASSWORD(:password))';
                $smt = $pdo->prepare($query);
                $smt->bindValue(':email', $this->email, PDO::PARAM_STR);
                $smt->bindValue(':password', $this->password, PDO::PARAM_STR);
        }

        $smt->execute();
        if(!$this->id) {
            $this->id = $pdo->lastInsertId();
        }
    }
}