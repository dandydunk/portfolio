<?php
class WingManInbox {
    public $id = 0;
	public $startUserId = 0;
    public $stage = 0;
	public $users = array();
	
	public function Approve($userId) {
		if($this->startUserId == $userId) {
			throw new Exception('you do not have permission.');
		}
		
		$pdo = WingManDB::GetConn();
        $query = "UPDATE inboxes SET stage=1 WHERE id=:inboxId";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
		
        $smt->execute();
	}
	
	public function IsUserValid($userId) {
		foreach($this->users as $user) {
			if($user == $userId) {
				return true;
			}
		}
		
		return false;
	}
	
	public function NewChatMessage($message, $fromUserId) {
		$pdo = WingManDB::GetConn();
        $query = "INSERT INTO inboxMessages(inboxId, content, timeCreated, fromUserId) 
					VALUES(:inboxId, :content, NOW(), :fromUserId)";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
		$smt->bindValue(':content', $message);
		$smt->bindValue(':fromUserId', $fromUserId, PDO::PARAM_INT);
		
        $smt->execute();
		
		$toUser = 0;
		if($this->users[0] == $fromUserId) {
			$toUser = $this->users[1];
		}
		else {
			$toUser = $this->users[0];
		}
		$user = WingManUser::GetById($toUser);
		if($user->isRobot) {
			$this->NewChatMessage("test message...", $toUser);
		}
	}
	
	public function Cancel($userId) {
		if($this->startUserId != $userId) {
			throw new Exception('you do not have permission.');
		}
		
		$pdo = WingManDB::GetConn();
        $query = "DELETE FROM inboxes WHERE id=:inboxId";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
		
        $smt->execute();
		
		$query = "DELETE FROM inboxUsers WHERE inboxId=:inboxId";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
		
        $smt->execute();
	}
	
	public function Reject($userId) {
		if($this->startUserId == $userId) {
			throw new Exception('you do not have permission.');
		}
		
		$pdo = WingManDB::GetConn();
		$query = "DELETE FROM inboxMessages WHERE inboxId=:inboxId";
        $smt = $pdo->prepare($query);
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
        $smt->execute();
		
		$query = "DELETE FROM inboxUsers WHERE inboxId=:inboxId";
        $smt = $pdo->prepare($query);
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
        $smt->execute();
		
		$query = "DELETE FROM inboxes WHERE id=:inboxId";
        $smt = $pdo->prepare($query);
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
        $smt->execute();
	}
	
	static public function GetById($id) {
		$pdo = WingManDB::GetConn();
        $query = "SELECT inboxes.id, inboxes.startUserId, inboxes.stage
				FROM inboxes
				WHERE id = :inboxId";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':inboxId', $id, PDO::PARAM_INT);
		
        $smt->execute();
        
        $resultInbox= $smt->fetch();
		if(!$resultInbox) {
			return null;
		}
		
		//
		$query = "SELECT userId
				FROM inboxUsers
				WHERE inboxId = :inboxId";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':inboxId', $id, PDO::PARAM_INT);
		
        $smt->execute();
        
		
		$inbox = new WingManInbox();
        $inbox->id = $id;
		$inbox->startUserId = $resultInbox['startUserId'];
		$inbox->stage = $resultInbox['stage'];
        while($row = $smt->fetch()) {
			array_push($inbox->users, $row['userId']);
		}
		
        return $inbox;
    }
	
    static public function GetByUsers($userOne, $userTwo) {
		$u = array($userOne->id, $userTwo->id);
		
		sort($u);
		
        $pdo = WingManDB::GetConn();
        $query = "SELECT inboxes.id, inboxes.startUserId, inboxes.stage, GROUP_CONCAT(inboxUsers.userId ORDER BY inboxUsers.userId) AS users
						FROM inboxes
						INNER JOIN inboxUsers ON inboxes.id=inboxUsers.inboxId
						GROUP BY inboxes.id
						HAVING users = '{$u[0]},{$u[1]}'";
        $smt = $pdo->prepare($query);
		
        //$smt->bindValue(':userOneId', (int)$u[0], PDO::PARAM_INT);
        //$smt->bindValue(':userTwoId', (int)$u[1], PDO::PARAM_INT);
		
        $smt->execute();
        
        $result = $smt->fetch();
		if(!$result) {
			return null;
		}
		
        $inbox = new WingManInbox();
        $inbox->id = $result['id'];
		$inbox->startUserId = $result['startUserId'];
		$inbox->stage = $result['stage'];
        
        return $inbox;
    }
	
	static public function GetInboxMessagesByInboxId($inboxId, $user) {
		$pdo = WingManDB::GetConn();
        $query = "SELECT
					inboxMessages.id, inboxMessages.inboxId, inboxMessages.content, inboxMessages.fromUserId, inboxMessages.timeCreated,inboxMessagesRead.userId 
					FROM inboxMessages
					LEFT JOIN inboxMessagesRead ON inboxMessagesRead.inboxMessageId = inboxMessages.id AND inboxMessagesRead.userId=:userId
					WHERE inboxMessagesRead.userId IS NULL AND inboxMessages.inboxId = :inboxId";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':inboxId', (int)$inboxId, PDO::PARAM_INT);
		$smt->bindValue(':userId', (int)$user->id, PDO::PARAM_INT);
        
        $smt->execute();
        
		$im = array();
        while($result = $smt->fetch()) {
			$im[] = $result;
		}
		
		return $im;
	}
	
	static public function GetNewInboxMessagesByUser($user) {
		$pdo = WingManDB::GetConn();
        $query = "SELECT inboxUsers.inboxId
				FROM inboxUsers
				WHERE inboxUsers.userId = :userId";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':userId', (int)$user->id, PDO::PARAM_INT);
        
        $smt->execute();
        
		$listMessages = array();
        while($result = $smt->fetch()) {
			$inbox = WingManInbox::GetById($result['inboxId']);
			$recipient = null;
			if($inbox->users[0] == $user->id) {
				$recipient = $inbox->users[1];
			}
			else {
				$recipient = $inbox->users[0];
			}
			
			$messages = WingManInbox::GetInboxMessagesByInboxId($result['inboxId'], $user);
			foreach($messages as $message) {
				$m = array();
				$m['message'] = $message['content'];
				$m['userId'] = $message['fromUserId'];
				$m['dateSent'] = $message['timeCreated'];
				$m['messageId'] = $message['id'];
				$m['inboxId'] = $message['inboxId'];
				$m['recipientUserId'] = $recipient;
				
				$listMessages[] = $m;
				WingManInbox::SetInboxMessageAsRead($message['id'], $user->id);
			}
		}
		
		return $listMessages;
    }
	
	static public function SetInboxMessageAsRead($messageId, $userId) {
		$pdo = WingManDB::GetConn();
        $query = "INSERT INTO inboxMessagesRead(inboxMessageId, userId, dateRead) VALUES(:messageId, :userId, NOW())";
        $smt = $pdo->prepare($query);
		
        $smt->bindValue(':userId', (int)$userId, PDO::PARAM_INT);
		$smt->bindValue(':messageId', (int)$messageId, PDO::PARAM_INT);
        
        $smt->execute();
	}
	
	static public function GetInboxesRequestsByUserId($userId) {
		$pdo = WingManDB::GetConn();
        $query = 'SELECT inboxes.id, inboxes.startUserId, inboxes.timeCreated FROM inboxes
				  INNER JOIN inboxUsers ON inboxUsers.inboxId = inboxes.id AND inboxes.startUserId != inboxUsers.userId
				  WHERE inboxes.stage=0 AND inboxUsers.userId = :userId';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':userId', $userId, PDO::PARAM_INT);
		
        
        $smt->execute();
		
		$result = array();
		while($row = $smt->fetch()) {
			$result[] = $row;
		}
		
		return $result;
	}
	
	static public function GetInboxesToAuthorizeByUserId($userId) {
		$pdo = WingManDB::GetConn();
        $query = 'SELECT id, userOneId, userTwoId FROM inboxes WHERE 
									(userOneId=:userId OR userTwoId=:userId) AND userStarterId != :userId AND stage=0';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':userId', $userId, PDO::PARAM_INT);
		
        
        $smt->execute();
		
		$result = array();
		while($row = $smt->fetch()) {
			$otherUserId = 0;
			if($row['userOneId'] == $userId) {
				$otherUserId = $row['userTwoId'];
			}
			else {
				$otherUserId = $row['userOneId'];
			}
			
			$result[] = array('inboxId'=>$row['id'],
							'otherUserId'=>$otherUserId);
		}
		
		return $result;
	}
    
    public function Save() {
        $inbox = WingManInbox::GetByUsers($this->userOne, $this->userTwo);
        if($inbox) {
            if(!$this->id) {
                $this->id = $inbox->id;
            }
            
            return;
        }
        
        $pdo = WingManDB::GetConn();
		
        $query = 'INSERT INTO inboxes(timeCreated, stage, startUserId) 
							VALUES(NOW(), :stage, :startUserId)';
        $smt = $pdo->prepare($query);
		$smt->bindValue(':startUserId', $this->userTwo->id, PDO::PARAM_INT);
		$smt->bindValue(':stage', ($this->userOne->isRobot || $this->userTwo->isRobot) ? 1:0, PDO::PARAM_INT);
		
        $smt->execute();
        
        if(!$this->id) {
            $this->id = $pdo->lastInsertId();
        }
		
		$query = 'INSERT INTO inboxUsers(inboxId, userId, starter) 
							VALUES(:inboxId, :userId, 0)';
        $smt = $pdo->prepare($query);
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
        $smt->bindValue(':userId', $this->userOne->id, PDO::PARAM_INT);
        
        $smt->execute();
		
		$query = 'INSERT INTO inboxUsers(inboxId, userId, starter) 
							VALUES(:inboxId, :userId, 1)';
		
        $smt = $pdo->prepare($query);
        $smt->bindValue(':inboxId', $this->id, PDO::PARAM_INT);
        $smt->bindValue(':userId', $this->userTwo->id, PDO::PARAM_INT);
        
        $smt->execute();
    }
}
