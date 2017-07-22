[6/24/15, 9:57:08 PM] marcus collins: okay..do you have brew installed?
[6/24/15, 9:57:10 PM] Thomas Yang: yes
[6/24/15, 9:57:24 PM] marcus collins: do brew update...then brew install mongodb
[6/24/15, 9:57:29 PM] Thomas Yang: i have mongodb
[6/24/15, 9:57:44 PM] marcus collins: is node and installed?
[6/24/15, 9:57:50 PM] Thomas Yang: yes
[6/24/15, 9:58:29 PM] marcus collins: okay open a terminal...and type mongo
[6/24/15, 9:58:37 PM] marcus collins: then type use staff_manager
[6/24/15, 9:58:59 PM] Thomas Yang: › mongo
MongoDB shell version: 3.0.4
connecting to: test
2015-06-24T21:58:45.116-0400 W NETWORK  Failed to connect to 127.0.0.1:27017, reason: errno:61 Connection refused
2015-06-24T21:58:45.118-0400 E QUERY    Error: couldn't connect to server 127.0.0.1:27017 (127.0.0.1), connection attempt failed
    at connect (src/mongo/shell/mongo.js:179:14)
    at (connect):1:6 at src/mongo/shell/mongo.js:179
exception: connect failed
[6/24/15, 9:59:25 PM] marcus collins: it lets you type?
[6/24/15, 9:59:30 PM] Thomas Yang: no
[6/24/15, 9:59:36 PM] Thomas Yang: it just fails and goes back to prompt
[6/24/15, 9:59:46 PM] marcus collins: oh, the server isn't running
[6/24/15, 9:59:51 PM] marcus collins: you know how to start it?
[6/24/15, 9:59:54 PM] Thomas Yang: no
[6/24/15, 9:59:59 PM] marcus collins: hold on
[6/24/15, 10:00:05 PM] marcus collins: i need to get the mac commands, i just had it
[6/24/15, 10:00:09 PM] Thomas Yang: mongod
[6/24/15, 10:00:10 PM] Thomas Yang: i think
[6/24/15, 10:00:22 PM] Thomas Yang: i opened another terminal and typed in mongod
[6/24/15, 10:00:25 PM] Thomas Yang: i think that worked
[6/24/15, 10:00:38 PM] marcus collins: oh yeah, theres a way to make it start when your os boots though
[6/24/15, 10:01:21 PM] marcus collins: but anyway type mongo now
[6/24/15, 10:01:24 PM] Thomas Yang: yea
[6/24/15, 10:01:26 PM] marcus collins: then use staff_manager
[6/24/15, 10:02:07 PM] marcus collins: then db.users.insert({email:"email@blah.com", password:"mypass", first_name:"thomas", last_name:"yang"})
[6/24/15, 10:02:31 PM] marcus collins: then type exit
[6/24/15, 10:02:43 PM] Thomas Yang: theres no seed file?
[6/24/15, 10:02:45 PM] Thomas Yang: lol
[6/24/15, 10:03:04 PM] marcus collins: its on my todo list
[6/24/15, 10:03:26 PM] marcus collins: okay in the directory type npm install
[6/24/15, 10:03:30 PM] marcus collins: wait
[6/24/15, 10:03:33 PM] marcus collins: sudo npm install
[6/24/15, 10:03:41 PM] marcus collins: then sudo npm update
[6/24/15, 10:04:00 PM] marcus collins: when it finishes start the server with sudo node server.js
[6/24/15, 10:04:09 PM] marcus collins: open a browser go to http://localhost:300
[6/24/15, 10:04:41 PM] Thomas Yang: it asks for passowrd
[6/24/15, 10:04:49 PM] Thomas Yang: oh nvm
[6/24/15, 10:04:53 PM] marcus collins: type email@blah.com / mypass
[6/24/15, 10:05:22 PM] Thomas Yang: connecting to database...
connected to database...
express-session deprecated undefined resave option; provide resave option server.js:42:10
express-session deprecated undefined saveUninitialized option; provide saveUninitialized option server.js:42:10
events.js:85
      throw er; // Unhandled 'error' event
            ^
Error: listen EACCES
    at exports._errnoException (util.js:746:11)
    at Server._listen2 (net.js:1139:19)
    at listen (net.js:1182:10)
    at Server.listen (net.js:1267:5)
    at EventEmitter.listen (/Users/tjy/Documents/staff_manager/node_modules/express/lib/application.js:617:24)
    at StartServer (/Users/tjy/Documents/staff_manager/server.js:61:19)
    at NativeConnection.<anonymous> (/Users/tjy/Documents/staff_manager/server.js:34:3)
    at NativeConnection.g (events.js:199:16)
    at NativeConnection.emit (events.js:104:17)
    at open (/Users/tjy/Documents/staff_manager/node_modules/mongoose/lib/connection.js:485:10)
    at NativeConnection.Connection.onOpen (/Users/tjy/Documents/staff_manager/node_modules/mongoose/lib/connection.js:494:5)
    at /Users/tjy/Documents/staff_manager/node_modules/mongoose/lib/connection.js:453:10
    at /Users/tjy/Documents/staff_manager/node_modules/mongoose/lib/drivers/node-mongodb-native/connection.js:59:5
    at /Users/tjy/Documents/staff_manager/node_modules/mongoose/node_modules/mongodb/lib/db.js:206:5
    at connectHandler (/Users/tjy/Documents/staff_manager/node_modules/mongoose/node_modules/mongodb/lib/server.js:272:7)
    at g (events.js:199:16)
[6/24/15, 10:05:31 PM] Thomas Yang: when i try to run node server js
[6/24/15, 10:05:56 PM] marcus collins: did oyu use sudo?
[6/24/15, 10:06:03 PM] marcus collins: sudo node server.js






[7/11/15, 4:09:34 PM] Thomas Yang: amazon ec2 right?
[7/11/15, 4:09:40 PM] marcus collins: yeah
[7/11/15, 4:09:52 PM] Thomas Yang: i choose ubuntu
[7/11/15, 4:09:55 PM] Thomas Yang: t2 micro
[7/11/15, 4:10:03 PM] Thomas Yang: review and launch or configure instance
[7/11/15, 4:10:18 PM] marcus collins: just launch it
[7/11/15, 4:10:33 PM] Thomas Yang: i need a new key pair
[7/11/15, 4:12:02 PM] marcus collins: http://the.earth.li/~sgtatham/putty/latest/x86/puttygen.exe
[7/11/15, 4:12:02 PM] Thomas Yang: ok i created one
[7/11/15, 4:12:06 PM] marcus collins: oh nm
[7/11/15, 4:12:18 PM] marcus collins: thats good for making keys also
[7/11/15, 4:12:34 PM] Thomas Yang: how do i invite you to make changes to my server
[7/11/15, 4:13:05 PM] marcus collins: well...send me the key, and i can access it via ssh to upload new files
[7/11/15, 4:13:21 PM] marcus collins: well..it can just pull from bitbucket though
[7/11/15, 4:13:34 PM] Thomas Yang: can you teach me how to do that
[7/11/15, 4:13:44 PM] marcus collins: its basically the same as on mac
[7/11/15, 4:13:52 PM] marcus collins: for ubuntu
[7/11/15, 4:14:00 PM] marcus collins: well first
[7/11/15, 4:14:07 PM] marcus collins: ssh to the server
[7/11/15, 4:14:12 PM] marcus collins: you know how to do that?
[7/11/15, 4:14:25 PM] marcus collins: open a terminal in mac os
[7/11/15, 4:14:34 PM] Thomas Yang: ok
[7/11/15, 4:14:42 PM] Thomas Yang: i have it
[7/11/15, 4:14:56 PM] marcus collins: actually wait a sec
[7/11/15, 4:15:33 PM] marcus collins: http://www.rackspace.com/knowledge_center/article/logging-in-with-a-ssh-private-key-on-linuxmac
[7/11/15, 4:16:00 PM] marcus collins: private key = the key you just made
[7/11/15, 4:17:27 PM] Thomas Yang: im in
[7/11/15, 4:18:49 PM] marcus collins: now to install node
[7/11/15, 4:18:52 PM] marcus collins: curl --silent --location https://deb.nodesource.com/setup_0.12 | sudo bash -
[7/11/15, 4:18:55 PM] marcus collins: then
[7/11/15, 4:19:02 PM] Thomas Yang: do i install on the SSH default folder?
[7/11/15, 4:19:14 PM] marcus collins: it will do it all for you
[7/11/15, 4:19:29 PM] marcus collins: copy/paste the curl command
[7/11/15, 4:19:30 PM] marcus collins: then
[7/11/15, 4:19:31 PM] marcus collins: sudo apt-get install --yes nodejs
[7/11/15, 4:19:51 PM] Thomas Yang: done
[7/11/15, 4:19:59 PM] marcus collins: ok..now to install mongoose
[7/11/15, 4:20:01 PM] marcus collins: hold on
[7/11/15, 4:20:41 PM] marcus collins: i mean mongo
[7/11/15, 4:21:25 PM] marcus collins: http://docs.mongodb.org/manual/tutorial/install-mongodb-on-ubuntu/
[7/11/15, 4:21:34 PM] marcus collins: scroll down to Install MongoDB
[7/11/15, 4:23:24 PM] Thomas Yang: how do i verify mongodb started succesffully
[7/11/15, 4:23:38 PM] marcus collins: type mongo
[7/11/15, 4:23:39 PM] Thomas Yang: Verify that MongoDB has started successfully
Verify that the mongod process has started successfully by checking the contents of the log file at /var/log/mongodb/mongod.log for a line reading
[7/11/15, 4:26:20 PM] Thomas Yang: how do i do that?
[7/11/15, 4:26:27 PM] Thomas Yang: Verify that the mongod process has started successfully by checking the contents of the log file at /var/log/mongodb/mongod.log for a line reading
[7/11/15, 4:26:28 PM] Thomas Yang: from ssh
[7/11/15, 4:26:43 PM] marcus collins: cat /var/log/mongodb/mongod.log
[7/11/15, 4:27:17 PM] marcus collins: but just type mongo
[7/11/15, 4:27:22 PM] marcus collins: if it connects, its all good
[7/11/15, 4:27:27 PM] Thomas Yang: yes
[7/11/15, 4:27:34 PM] Thomas Yang: now what
[7/11/15, 4:28:57 PM] marcus collins: do mkdir /var/staffmanager
[7/11/15, 4:29:05 PM] marcus collins: or some other name instead of staffmanager
[7/11/15, 4:29:18 PM] marcus collins: then cd /var/staffmanager
[7/11/15, 4:29:28 PM] Thomas Yang: done
[7/11/15, 4:29:30 PM] Thomas Yang: var/domotaco
[7/11/15, 4:29:38 PM] Thomas Yang: whats the var folder
[7/11/15, 4:29:44 PM] marcus collins: cd /var/domotaco/
[7/11/15, 4:30:13 PM] marcus collins: variable data files
[7/11/15, 4:30:15 PM] Thomas Yang: ok
[7/11/15, 4:30:22 PM] Thomas Yang: im in
[7/11/15, 4:30:31 PM] marcus collins: git pull https://macollins2012@bitbucket.org/tjy86/domo-taco.git
[7/11/15, 4:30:37 PM] marcus collins: oops
[7/11/15, 4:30:39 PM] marcus collins: get your link
[7/11/15, 4:30:43 PM] marcus collins: to use your pass
[7/11/15, 4:31:29 PM] Thomas Yang: fatal: Not a git repository (or any of the parent directories): .git
[7/11/15, 4:31:42 PM] marcus collins: git init
[7/11/15, 4:31:45 PM] marcus collins: then git pull
[7/11/15, 4:32:00 PM] marcus collins: tht part is the same as you did on mac
[7/11/15, 4:32:03 PM] Thomas Yang: git pull https://tjy86@bitbucket.org/tjy86/domo-taco.git
error: cannot open .git/FETCH_HEAD: Permission denied
[7/11/15, 4:32:12 PM] marcus collins: hmm hold on
[7/11/15, 4:33:23 PM] marcus collins: oh wait
[7/11/15, 4:33:26 PM] marcus collins: git clone
[7/11/15, 4:33:49 PM] Thomas Yang: git clone https://tjy86@bitbucket.org/tjy86/domo-taco.git
fatal: could not create work tree dir 'domo-taco'.: Permission denied
[7/11/15, 4:33:55 PM] marcus collins: ug sec
[7/11/15, 4:37:01 PM] marcus collins: try
[7/11/15, 4:37:09 PM] marcus collins: putting sudo in front
[7/11/15, 4:38:00 PM] Thomas Yang: done
[7/11/15, 4:38:14 PM] marcus collins: okay... sudo node server.js
[7/11/15, 4:38:36 PM] Thomas Yang:  sudo node server.js
module.js:338
    throw err;
          ^
Error: Cannot find module '/var/domotaco/server.js'
    at Function.Module._resolveFilename (module.js:336:15)
    at Function.Module._load (module.js:278:25)
    at Function.Module.runMain (module.js:501:10)
    at startup (node.js:129:16)
    at node.js:814:3
[7/11/15, 4:38:43 PM] Thomas Yang: ah
[7/11/15, 4:38:48 PM] Thomas Yang: made a folder in folder
[7/11/15, 4:39:57 PM] Thomas Yang: ok
[7/11/15, 4:40:04 PM] Thomas Yang: how do i connect?
[7/11/15, 4:40:50 PM] marcus collins: sec...loading my ubuntu up..need to chage the port to 80
[7/11/15, 4:41:02 PM] marcus collins: so you wont need to do any port fowarding..or still may
[7/11/15, 4:41:14 PM] Thomas Yang: udo node server.js
connecting to database...
connected to database...
express-session deprecated undefined resave option; provide resave option server.js:60:10
express-session deprecated undefined saveUninitialized option; provide saveUninitialized option server.js:60:10
listening at http://:::300
[7/11/15, 4:41:19 PM] Thomas Yang: when do we work on the interface
[7/11/15, 4:41:40 PM] marcus collins: you can find a template
[7/11/15, 4:41:57 PM] marcus collins: i can do that during the week while youre bug testing
[7/11/15, 4:44:07 PM] marcus collins: what is the ip address of the instance?
[7/11/15, 4:44:36 PM] Thomas Yang: 172.31.61.177
[7/11/15, 4:45:43 PM] marcus collins: http://stackoverflow.com/questions/5004159/opening-port-80-ec2-amazon-web-services/10454688#10454688
[7/11/15, 4:45:56 PM] marcus collins: fllow those steps, but enter 300 for now instead of 80
[7/11/15, 4:46:54 PM] Thomas Yang: not going
[7/11/15, 4:47:06 PM] marcus collins: not going where?
[7/11/15, 4:47:12 PM] Thomas Yang: i mean not loading
[7/11/15, 4:47:16 PM] Thomas Yang: the 172.31.61.177:300
[7/11/15, 4:47:18 PM] marcus collins: you will have to restart the instance
[7/11/15, 4:47:22 PM] marcus collins: after u update that
[7/11/15, 4:47:25 PM] Thomas Yang: i didn't update
[7/11/15, 4:47:29 PM] Thomas Yang: I'm trying to go 300
[7/11/15, 4:47:30 PM] Thomas Yang: port 300
[7/11/15, 4:47:43 PM] marcus collins: you gotta follow those steps in the link to open the port
[7/11/15, 4:47:58 PM] marcus collins: they block all ports
[7/11/15, 4:48:03 PM] marcus collins: you gotta open them manually
[7/11/15, 4:48:10 PM] Thomas Yang: ok
[7/11/15, 4:50:07 PM] Thomas Yang: will it be hard to make them use camera while clocking in?
[7/11/15, 4:51:02 PM] marcus collins: il have to add it
[7/11/15, 4:51:08 PM] Thomas Yang: i know
[7/11/15, 4:51:14 PM] Thomas Yang: but is it hard considering the device could be
[7/11/15, 4:51:18 PM] Thomas Yang: android, ipad, iphone
[7/11/15, 4:51:26 PM] Thomas Yang: or android tablet
[7/11/15, 4:51:56 PM] marcus collins: it should take about an hour to code or so
[7/11/15, 4:52:25 PM] marcus collins: release the funds today, ill do it for free
[7/11/15, 4:52:34 PM] Thomas Yang: i can't change the
[7/11/15, 4:52:36 PM] Thomas Yang: port to 300
[7/11/15, 4:52:42 PM] Thomas Yang: when i select HTTP
[7/11/15, 4:53:04 PM] marcus collins: is there something there to enter a custom port?
[7/11/15, 4:53:21 PM] Thomas Yang: is it TCP?
[7/11/15, 4:53:22 PM] marcus collins: i can't see the interface...im going off memory and google
[7/11/15, 4:53:24 PM] marcus collins: yeah tcp
[7/11/15, 4:53:27 PM] Thomas Yang: ok
[7/11/15, 4:53:40 PM] Thomas Yang: rebooting instance
[7/11/15, 4:54:05 PM] Thomas Yang: do i have to SSH again and start server?
[7/11/15, 4:54:25 PM] marcus collins: yeah for now, ill make a script today to have it start automatically..i gotta google how
[7/11/15, 4:54:52 PM] Thomas Yang: isn't sudo node server a bit risky?
[7/11/15, 4:55:15 PM] Thomas Yang: http://172.31.61.177:300/
[7/11/15, 4:55:17 PM] Thomas Yang: still not going
[7/11/15, 4:55:27 PM] Thomas Yang: ubuntu@ip-172-31-61-177:~$ cd /var/domo-taco/
ubuntu@ip-172-31-61-177:/var/domo-taco$ sudo node server.js
connecting to database...
connected to database...
express-session deprecated undefined resave option; provide resave option server.js:60:10
express-session deprecated undefined saveUninitialized option; provide saveUninitialized option server.js:60:10
listening at http://:::300
[7/11/15, 4:55:31 PM] marcus collins: what do you mean by risky
[7/11/15, 4:55:43 PM] Thomas Yang: the whole point is not to do SUDO
[7/11/15, 4:55:44 PM] Thomas Yang: irhgt
[7/11/15, 4:55:46 PM] Thomas Yang: right*
[7/11/15, 4:55:47 PM] marcus collins: hmm port didnt open..hold on, let me see
[7/11/15, 4:56:07 PM] marcus collins: no one can do anything to do the server from that
[7/11/15, 4:56:51 PM] Thomas Yang: got it
[7/11/15, 4:56:52 PM] Thomas Yang: http://52.2.17.152:300/account/login
[7/11/15, 4:56:53 PM] Thomas Yang: public ip
[7/11/15, 4:56:55 PM] Thomas Yang: is different
[7/11/15, 4:57:05 PM] marcus collins: cool :)
[7/11/15, 4:57:18 PM] Thomas Yang: login failed
[7/11/15, 4:57:19 PM] marcus collins: now...to run the server forever, when u log ogg
[7/11/15, 4:57:21 PM] marcus collins: just a sec
[7/11/15, 4:57:45 PM] marcus collins: you have to create a new admin user...  node new_admin.js 123456789
[7/11/15, 4:58:00 PM] marcus collins: or some other pass
[7/11/15, 4:58:06 PM] Thomas Yang: limited to 4 digits or unlimited
[7/11/15, 4:58:15 PM] marcus collins: i didnt set a limit
[7/11/15, 4:59:35 PM] marcus collins: so you will want to set their picture to whatever its taken?
[7/11/15, 4:59:48 PM] marcus collins: in the admin
[7/11/15, 4:59:53 PM] marcus collins: or do you wanna use that for soemthing else
[7/11/15, 5:00:01 PM] marcus collins: i made it so you canupload employee's pictures
[7/11/15, 5:00:21 PM] Thomas Yang: like
[7/11/15, 5:00:24 PM] Thomas Yang: i want to make sure its my employee that is clocking in
[7/11/15, 5:01:31 PM] marcus collins: oh yeah..i could do face recognition later if you want..but it wont be too cheap
[7/11/15, 5:01:53 PM] marcus collins: you would have to upload a picture of them for me to analyze with one their taking
[7/11/15, 5:02:12 PM] Thomas Yang: i don't need facial recognition
[7/11/15, 5:02:19 PM] Thomas Yang: i just want a picture taken and logged to the clock in and out db
[7/11/15, 5:02:30 PM] marcus collins: cool
[7/11/15, 5:02:56 PM] marcus collins: stop the server
[7/11/15, 5:03:15 PM] Thomas Yang: why
[7/11/15, 5:03:22 PM] marcus collins: type sudo npm install forever -g
[7/11/15, 5:03:31 PM] marcus collins: so we can get it running even when u log out
[7/11/15, 5:03:42 PM] marcus collins: i mean keep it running after you log out
[7/11/15, 5:03:47 PM] Thomas Yang: i see
[7/11/15, 5:03:53 PM] Thomas Yang: how do i reset it
[7/11/15, 5:03:59 PM] Thomas Yang: or stop/start it
[7/11/15, 5:04:03 PM] marcus collins: ctrl c
[7/11/15, 5:04:11 PM] Thomas Yang: i mean with the forever
[7/11/15, 5:04:23 PM] marcus collins: forever start server.js
[7/11/15, 5:04:26 PM] marcus collins: forever stop server.js
[7/11/15, 5:04:28 PM] Thomas Yang: cool
[7/11/15, 5:04:56 PM] Thomas Yang: warn:    --minUptime not set. Defaulting to: 1000ms
warn:    --spinSleepTime not set. Your script will exit if it does not stay up for at least 1000ms
info:    Forever processing file: serv
[7/11/15, 5:05:14 PM] Thomas Yang: connection refused right now
[7/11/15, 5:05:25 PM] marcus collins: type sudo forever
[7/11/15, 5:05:41 PM] Thomas Yang: got it
[7/11/15, 5:08:35 PM] Thomas Yang: ok. the dashboard is not telling me how much he's making
[7/11/15, 5:08:39 PM] Thomas Yang: 1001 admin
[7/11/15, 5:09:38 PM] marcus collins: http://52.2.17.152:300/employee/55a183beb0d1b3e50441d0af/
[7/11/15, 5:09:45 PM] marcus collins: pay rate
