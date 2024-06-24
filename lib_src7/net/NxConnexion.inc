<?php

class NxConnexion {
	// close active connection, allowing a slow script to continue in the background in asynchronous
	static function close($shutdownFunction=null) {
		while(ob_get_level()) ob_end_clean();
    	flush();

		header('Connection: close');
		ignore_user_abort();
		ob_start();
		echo('Connection Closed');
		$size = ob_get_length();
		header("Content-Length: $size");
		ob_end_flush();

		flush();

		if($shutdownFunction)
			register_shutdown_function($shutdownFunction);
	}
}

?>