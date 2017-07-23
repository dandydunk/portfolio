<?php
ob_start('ob_gzhandler');

require 'inc/config.php';
session_start();

if(!isset($_REQUEST['action'])) {
    ErrorMsg('the action is missing.');
    exit;
}


switch(trim($_REQUEST['action'])) {
	case 'fbNewAccount':
		if(!isset($_REQUEST['accessToken']) || !strlen(trim($_REQUEST['accessToken']))) {
			ErrorMsg('access token is missing.');
			exit;
		}
		
		require_once 'inc\php-graph-sdk-5.0.0\src\Facebook\autoload.php';
		
		$fb = new Facebook\Facebook(['app_id' => '1786091621707514',
									  'app_secret' => '59fb75ad9f5a6047ad1a14cf1b89b906',
									  'default_graph_version' => 'v2.5'
									]);
		$fb->setDefaultAccessToken($_REQUEST['accessToken']);

		try {
		  $response = $fb->get('/me?fields=email');
		  $userNode = $response->getGraphUser();
		} catch(Facebook\Exceptions\FacebookResponseException $e) {
		   ErrorMsg('Internal error, contact admin.');
		  //echo 'Graph returned an error: ' . $e->getMessage();
		  exit;
		} catch(Facebook\Exceptions\FacebookSDKException $e) {
		  ErrorMsg('Internal error, contact admin.');
		  //echo 'Facebook SDK returned an error: ' . $e->getMessage();
		  exit;
		}
		
		$email = $userNode['email'];
		$password = 'testsjsjs';
		
		$wm = WingManUser::GetByEmail($email);
		if($wm) {
			ErrorMsg('there is already an account registered to your email');
			exit;
		}
		
		$wm = new WingManUser();
        $wm->email = $email;
        $wm->password = $password;
        try {
            $wm->Save();
            $wm->CreateProfile();
        } catch(Exception $ex) {
            ErrorMsg($ex->getMessage());
            exit;
        }
        
        $_SESSION['userId'] = $wm->id;
        $wm->sessionId = session_id();
        $wm->UpdateSessionId();
        echo json_encode(array('sessionId'=>$wm->sessionId, 'userId'=>$wm->id));
	exit;
	
	case 'fbLogin':
		if(!isset($_REQUEST['accessToken']) || !strlen(trim($_REQUEST['accessToken']))) {
			ErrorMsg('access token is missing.');
			exit;
		}
		
		require_once 'inc\php-graph-sdk-5.0.0\src\Facebook\autoload.php';
		
		$fb = new Facebook\Facebook(['app_id' => '1786091621707514',
									  'app_secret' => '59fb75ad9f5a6047ad1a14cf1b89b906',
									  'default_graph_version' => 'v2.5'
									]);
		$fb->setDefaultAccessToken($_REQUEST['accessToken']);

		try {
		  $response = $fb->get('/me?fields=email');
		  $userNode = $response->getGraphUser();
		} catch(Facebook\Exceptions\FacebookResponseException $e) {
		   ErrorMsg('Internal error, contact admin.');
		  //echo 'Graph returned an error: ' . $e->getMessage();
		  exit;
		} catch(Facebook\Exceptions\FacebookSDKException $e) {
		  ErrorMsg('Internal error, contact admin.');
		  //echo 'Facebook SDK returned an error: ' . $e->getMessage();
		  exit;
		}
		
		$user = WingManUser::GetByEmail($userNode['email']);
		if(!$user) {
			ErrorMsg('the user does not exist for that email.');
			exit;
		}
		
		$_SESSION['userId'] = $user->id;
		$user->sessionId = session_id();
		$user->UpdateSessionId();
		echo json_encode(array('sessionId'=>$user->sessionId, 'userId'=>$user->id));
	exit;
	
	case 'test':
		$coords['latitude'] = 47.691277;
		$coords['longitude'] = -122.189941;
		GoogleAPI::GPSToAddress($coords);
	exit;
	
    case 'makeAccounts':
        for($i = 0; $i < 100; $i++) {
            $wm = new WingManUser();
            $wm->email = "email{$i}@gmail.com";
            $wm->password = 'pass';
            try {
                $wm->Save();
                $wm->CreateProfile2("FName{$i}","LName{$i}");
            } catch(Exception $ex) {
                ErrorMsg($ex->getMessage());
                exit;
            }
        }
    exit;
        
    case 'newAccount':
        if(!isset($_REQUEST['email']) || !strlen(trim($_REQUEST['email']))) {
            ErrorMsg('the email is missing.');
            exit;
        }

        if(!isset($_REQUEST['password']) || !strlen(trim($_REQUEST['password']))) {
            ErrorMsg('the password is missing.');
            exit;
        }

        if(!isset($_REQUEST['passwordConfirm']) || strcmp($_REQUEST['password'], $_REQUEST['passwordConfirm'])) {
            ErrorMsg('the passwords do not match.');
            exit;
        }

        $wm = new WingManUser();
        $wm->email = $_REQUEST['email'];
        $wm->password = $_REQUEST['password'];
        try {
            $wm->Save();
            $wm->CreateProfile();
        } catch(Exception $ex) {
            ErrorMsg($ex->getMessage());
            exit;
        }
        
        $_SESSION['userId'] = $wm->id;
        $wm->sessionId = session_id();
        $wm->UpdateSessionId();
        echo json_encode(array('sessionId'=>$wm->sessionId, 'userId'=>$wm->id));
    exit;

    case 'login':
        if(!isset($_REQUEST['email']) || !strlen(trim($_REQUEST['email']))) {
            ErrorMsg('the email is missing.');
            exit;
        }

        if(!isset($_REQUEST['password']) || !strlen(trim($_REQUEST['password']))) {
            ErrorMsg('the password is missing.');
            exit;
        }
        
        $wm = new WingManUser();
        $wm->email = $_REQUEST['email'];
        $wm->password = $_REQUEST['password'];
        
        if($wm->Validate()) {
            $_SESSION['userId'] = $wm->id;
            $wm->sessionId = session_id();
            $wm->UpdateSessionId();
            echo json_encode(array('sessionId'=>$wm->sessionId, 'userId'=>$wm->id));
            exit;
        }
        
        ErrorMsg('incorrect email or password.');
    exit;
    
    case 'getProfileValues':
        if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
        
        $wu->FillProfile();
        
        echo json_encode(array('userName'=>$wu->userName,
                                'birthday'=>$wu->birthday));
    exit;
    
    case 'getUserProfile':
        if(!isset($_REQUEST['userId']) || !strlen(trim($_REQUEST['userId']))) {
            ErrorMsg('the user id is missing.');
            exit;
        }
        
        $wu = new WingManUser();
        $wu->id = $_REQUEST['userId'];
        $wu->FillProfile();
		
		$pic = $wu->GetProfilePhotoBytes();
		if(!$pic) {
			$pic = base64_encode(file_get_contents('ext/noImg.gif'));
		}
        
        echo json_encode(array('userName'=>$wu->userName,
                                'profilePic'=> $pic
								));
    exit;
	
	case 'getLocation':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		$location = $wu->GetLocation();
		if($location) {
			echo json_encode($location);
		}
		else {
			ErrorMsg2('no location set', 102);
            exit;
		}
	exit;
    
	case 'saveCheckIn':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['latitude'])) {
			ErrorMsg('the latitude is missing.');
            exit;
		}
		
		if(!isset($_REQUEST['longitude'])) {
			ErrorMsg('the longitude is missing.');
            exit;
		}
		
		if(!isset($_REQUEST['placeName'])) {
			ErrorMsg('the place name is missing.');
            exit;
		}
		
		$coords['latitude'] = $_REQUEST['latitude'];
		$coords['longitude'] = $_REQUEST['longitude'];
		$coords['placeName'] = $_REQUEST['placeName'];
		
		$wu->CheckIn($coords);
		Success();
	exit;
	
    case 'getUsersByRadius':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
        $users = $wu->GetByRadius();
        echo json_encode($users);
    exit;
    
    case 'getPhotosBySession':
        if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
        
        $pics = $wu->GetPhotos();
        echo json_encode($pics);
    exit;
    
    case 'deletePhoto':
        if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
        
        if(!isset($_REQUEST['photoId'])) {
            ErrorMsg('the photo id is missing.');
            exit;
        }
        
        $wu->DeletePhoto($_REQUEST['photoId']);
        Success();
    exit;
    
    case 'uploadPhoto':
        if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
        
        if(!isset($_REQUEST['photoBytes'])) {
            ErrorMsg('invalid photo bytes');
            exit;
        }
        
        $wu->SavePhoto(rawurldecode($_REQUEST['photoBytes']));
        Success();
    exit;
    
    case 'updateProfile':
        if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
        
		if(isset($_REQUEST['userName'])) {
			$uu = WingManUser::GetByUserName($_REQUEST['userName']);
			if($uu) {
				ErrorMsg('the user name already exists.');
				exit;
			}
            $wu->UpdateProfileValue('userName', trim($_REQUEST['userName']));
        }
        
        if(isset($_REQUEST['firstName'])) {
            $wu->UpdateProfileValue('firstName', trim($_REQUEST['firstName']));
        }
        
        if(isset($_REQUEST['lastName'])) {
            $wu->UpdateProfileValue('lastName', trim($_REQUEST['lastName']));
        }
        
        if(isset($_REQUEST['birthday'])) {
            $wu->UpdateProfileValue('birthday', trim($_REQUEST['birthday']));
        }
        
        if(isset($_REQUEST['city'])) {
            $wu->UpdateProfileValue('city', trim($_REQUEST['city']));
        }
        
        if(isset($_REQUEST['state'])) {
            $wu->UpdateProfileValue('state', trim($_REQUEST['state']));
        }
        
        if(isset($_REQUEST['zip'])) {
            $wu->UpdateProfileValue('zip', trim($_REQUEST['zip']));
        }
        
        Success();
    exit;
	
	case 'inboxAuthorizations':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		$results = WingManInbox::GetInboxesToAuthorizeByUserId($wu->id);
		return json_encode($results);
	exit;
	
	case 'approveInbox':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['inboxId']) || !strlen(trim($_REQUEST['inboxId']))) {
            ErrorMsg('the inbox id is missing.');
            exit;
        }
		
		$inbox = WingManInbox::GetById($_REQUEST['inboxId']);
		if(!$inbox) {
			ErrorMsg('the inbox id is invalid.');
            exit;
		}
		
		try {
			$inbox->Approve($wu->id);
		} catch(Exception $ex) {
			ErrorMsg($ex->getMessage());
            exit;
		}
		
		Success();
	exit;
	
	case 'rejectInbox':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['inboxId']) || !strlen(trim($_REQUEST['inboxId']))) {
            ErrorMsg('the inbox id is missing.');
            exit;
        }
		
		$inbox = WingManInbox::GetById($_REQUEST['inboxId']);
		if(!$inbox) {
			ErrorMsg('the inbox id is invalid.');
            exit;
		}
		
		try {
			$inbox->Reject($wu->id);
		} catch(Exception $ex) {
			ErrorMsg($ex->getMessage());
            exit;
		}
		
		Success();
	exit;
	
	case 'cancelInbox':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['inboxId']) || !strlen(trim($_REQUEST['inboxId']))) {
            ErrorMsg('the inbox id is missing.');
            exit;
        }
		
		$inbox = WingManInbox::GetById($_REQUEST['inboxId']);
		if(!$inbox) {
			ErrorMsg('the inbox id is invalid.');
            exit;
		}
		try {
			$inbox->Cancel($wu->id);
		} catch(Exception $ex) {
			ErrorMsg($ex->getMessage());
            exit;
		}
		
		Success();
	exit;
	
	case 'getInboxOtherUser':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
		
		$wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['inboxId']) || !strlen(trim($_REQUEST['inboxId']))) {
            ErrorMsg('the inbox id is missing.');
            exit;
        }
		
		$inbox = WingManInbox::GetById($_REQUEST['inboxId']);
		if(!$inbox) {
			ErrorMsg('invalid inbox id');
            exit;
		}
		
		$u = 0;
		if($inbox->users[0] == $wu->id) {
			$u = $inbox->users[1];
		}
		else {
			$u = $inbox->users[0];
		}
		
		echo json_encode(array(
		'userId'=>$u
		));
	exit;
	
	case 'getInbox':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
		
		$wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['toUserId']) || !strlen(trim($_REQUEST['toUserId']))) {
            ErrorMsg('the to user id is missing.');
            exit;
        }
		
		$toUser = WingManUser::GetById(trim($_REQUEST['toUserId']));
		if(!$toUser) {
			ErrorMsg('the to user id is invalid.');
            exit;
		}
		
		$inbox = WingManInbox::GetByUsers($wu, $toUser);
		if(!$inbox) {
			ErrorMsg2('no inbox', 100);
            exit;
		}
		
		echo json_encode(array(
		'inboxId'=>$inbox->id, 
		'stage'=>$inbox->stage, 
		'startUserId'=>$inbox->startUserId
		));
	exit;
	
	case 'getNewInboxMessages':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
		
		$wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		$inbox = WingManInbox::GetNewInboxMessagesByUser($wu);
		if(!count($inbox)) {
			ErrorMsg2('no inbox messages', 100);
            exit;
		}
		
		echo json_encode($inbox);
	exit;
	
	case 'newRequests':
		if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		$requests = WingManInbox::GetInboxesRequestsByUserId($wu->id);
		if(count($requests)) {
			echo(json_encode($requests));
		}
		else {
			ErrorMsg2('no requests', 101);
            exit;
		}
	exit;
    
    case 'newInbox':
        if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['toUserId']) || !strlen(trim($_REQUEST['toUserId']))) {
            ErrorMsg('the to user id is missing.');
            exit;
        }
		
		$toUser = WingManUser::GetById(trim($_REQUEST['toUserId']));
		if(!$toUser) {
			ErrorMsg('the to user id is invalid.');
            exit;
		}
		
		$inbox = WingManInbox::GetByUsers($wu, $toUser);
		if($inbox) {
			ErrorMsg2('conversation already exists', 100);
            exit;
		}
		
		$inbox = new WingManInbox();
		$inbox->userOne = $toUser;
		$inbox->userTwo = $wu;
		$inbox->userStarter = $wu;
		$inbox->Save();
		
		echo(json_encode(array('inboxId'=>$inbox->id)));
    exit;
	
	case 'newMessage':
        if(!isset($_REQUEST['sessionId']) || !strlen(trim($_REQUEST['sessionId']))) {
            ErrorMsg('the session id is missing.');
            exit;
        }
        
        $wu = WingManUser::GetBySessionId($_REQUEST['sessionId']);
        if(!$wu) {
            ErrorMsg('invalid session id');
            exit;
        }
		
		if(!isset($_REQUEST['chatMessage']) || !strlen(trim($_REQUEST['chatMessage']))) {
            ErrorMsg('the chat message is missing.');
            exit;
        }
		
		if(!isset($_REQUEST['inboxId']) || !strlen(trim($_REQUEST['inboxId']))) {
            ErrorMsg('the inbox id is missing.');
            exit;
        }
		
		$inbox = WingManInbox::GetById(trim($_REQUEST['inboxId']));
		if(!$inbox) {
			ErrorMsg('the inbox id is invalid.');
            exit;
		}
		
		if(!$inbox->IsUserValid($wu->id)) {
			ErrorMsg('you do not have permission.');
            exit;
		}
		
		$inbox->NewChatMessage(trim($_REQUEST['chatMessage']), $wu->id);
		
		Success();
    exit;
	
	default:
		ErrorMsg('invalid Action '.$_REQUEST['action']);
	exit;
}

function ErrorMsg($txt) {
    echo json_encode(array('error'=>$txt));
}

function ErrorMsg2($txt, $num) {
    echo json_encode(array('error'=>$txt, 'errorNumber'=>$num));
}

function Success() {
    echo json_encode(array('success'=>true));
}

