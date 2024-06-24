<?php

$properties= array(

// operations/steps

// newUser.start
"register.newUser.start.submit.redirect" => "submit",
"register.newUser.start.cancel.redirect" => "cancel",

"view.register.newUser.start" => $this->getProperty('page.edit.view'),
"db.register.newUser.start" => $this->getProperty('user.register.db','registration'),
"register.newUser.start.submit.redirect" => 'ok',
"register.newUser.start.cancel.redirect" => 'cancel',

// newUser.confirm
"view.register.newUser.confirm" => $this->getProperty('page.edit.view','registration'),
"db.register.newUser.confirm" => $this->getProperty('user.register.db','registration'),
"db.register.newUser.confirm.checkid" => $this->getProperty('user.check.db'),
"register.newUser.list.countrycode" => "country_code",
"register.newUser.confirm.msg" => "Please confirm your details:",
"register.newUser.confirm.modify.redirect" => "modify",
"register.newUser.confirm.confirm.redirect" => "submit",
"register.newUser.confirm.cancel.label" => "cancel",
"register.newUser.confirm.submit.redirect" => "ok",
"register.newUser.confirm.cancel.redirect" => "cancel",

// newUser.modify
"view.register.newUser.modify" => $this->getProperty('page.edit.view','registration'),
"db.register.newUser.modify" => $this->getProperty('user.register.db',"registration"),
"register.newUser.modify.msg" => "Please confirm your details:",
"register.newUser.modify.submit.redirect" => "ok",
"register.newUser.modify.cancel.redirect" => "cancel",

// newUser.send
"view.register.newUser.send" => $this->getProperty('page.edit.view','registration'),
"db.register.newUser.complete" => $this->getProperty('user.update.db'),
"register.newUser.send.msg" => "Please confirm your details:",
"register.newUser.send.submit.redirect" => "ok",
"register.newUser.send.cancel.modify" => "cancel",

// email sending error dlg
"register.newUser.email.cancel.redirect" => "cancel",
"register.newUser.email.checkEmail.label" => "try again",
"register.newUser.email.cancelURL" => "/",

// view for email
//"view.register.newUser.email.html" => "view.email",
"view.register.newUser.email.text" => "email.registration_text",
"register.newUser.email.subject" => "Registration details for NxMed32.com web site",
"register.newUser.email.from" => "noreply@nxmed32.com",
"db.register.newUser.email"   => "registration_email",

// check email and login form
"view.register.newUser.login" => $this->getProperty('page.edit.view','registration'),
"db.register.newUser.login" => $this->getProperty('user.register.db',"registration"),
"register.newUser.login.msg" => "check your email",
"register.newUser.login.submit.redirect" => "ok",
"register.newUser.login.cancel.redirect" => "cancel",

// newUser.validateLogin
"view.register.newUser.validateLogin" => $this->getProperty('page.edit.view','registration'),
"db.register.newUser.validateLogin" => $this->getProperty('user.login.db'),

"register.newUser.validateLogin.msg" => "Please confirm your details:",
"register.newUser.validateLogin.submit.redirect" => "ok",
"register.newUser.validateLogin.cancel.redirect" => "cancel",	

// database saving
"db.register.newUser.save" => $this->getProperty('user.save.db'),
"register.newUser.save.msg" => "Congratulation, your account is now created.<br> Please click following button to enter your new home page.",
"register.newUser.save.ok.label" => "Home",
"register.newUser.save.ok.redirect" => "ok",
"register.newUser.save.ok.url" => $this->getProperty('user.home.url'),

"view.register.newUser.errorlogin" => $this->getProperty('page.edit.view','registration'),
"db.register.newUser.errorlogin" => "login",
"register.newUser.errorlogin.msg" => "Error in user id or password, please try again",
"register.newUser.errorlogin.submit.redirect" => "ok",	
"register.newUser.errorlogin.cancel.redirect" => "cancel",	

// generic views
"view.message" => "page.message",
"view.errorEditRecord" => $this->getProperty('page.edit.view','registration'),

);
?>
