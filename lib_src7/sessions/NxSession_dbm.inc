<?
/* ------------------------------------------------------------------------
 * session_dbm.php
 * ------------------------------------------------------------------------
 * PHP4 DBM Session Handler
 * Version 1.00
 * by Ying Zhang (ying@zippydesign.com)
 * Last Modified: May 21 2000
 *
 * ------------------------------------------------------------------------
 * TERMS OF USAGE:
 * ------------------------------------------------------------------------
 * You are free to use this library in any way you want, no warranties are
 * expressed or implied.  This works for me, but I don't guarantee that it
 * works for you, USE AT YOUR OWN RISK.
 *
 * While not required to do so, I would appreciate it if you would retain
 * this header information.  If you make any modifications or improvements,
 * please send them via email to Ying Zhang <ying@zippydesign.com>.
 *
 * ------------------------------------------------------------------------
 * DESCRIPTION:
 * ------------------------------------------------------------------------
 * This library tells the PHP4 session handler to write to a DBM file
 * instead of creating individual files for each session.
 *
 * ------------------------------------------------------------------------
 * INSTALLATION:
 * ------------------------------------------------------------------------
 * Make sure you have DBM support compiled into PHP4.  Then copy this
 * script to a directory that is accessible by the rest of your PHP
 * scripts.
 *
 * ------------------------------------------------------------------------
 * USAGE:
 * ------------------------------------------------------------------------
 * Include this file in your scripts before you call session_start(), you
 * don't have to do anything special after that.
 */

$SESS_DBM = "";
$SESS_LIFE = get_cfg_var("session.gc_maxlifetime");

function sess_open($save_path, $session_name) {
	global $SESS_DBM;

	$SESS_DBM = dbmopen("$save_path/$session_name", "c");
	return ($SESS_DBM);
}

function sess_close() {
	global $SESS_DBM;

	dbmclose($SESS_DBM);
	return true;
}

function sess_read($key) {
	global $SESS_DBM, $SESS_LIFE;

	$var = "";
	if ($tmp = dbmfetch($SESS_DBM, $key)) {
		$expires_at = substr($tmp, 0, strpos($tmp, "|"));

		if ($expires_at > time()) {
			$var = substr($tmp, strpos($tmp, "|") + 1);
		}
	}

	return $var;
}

function sess_write($key, $val) {
	global $SESS_DBM, $SESS_LIFE;

	dbmreplace($SESS_DBM, $key, time() + $SESS_LIFE . "|" . $val);
	return true;
}

function sess_destroy($key) {
	global $SESS_DBM;

	dbmdelete($SESS_DBM, $key);
	return true;
}

function sess_gc($maxlifetime) {
	global $SESS_DBM;

	$now = time();
	$key = dbmfirstkey($SESS_DBM);
	while ($key) {
		if ($tmp = dbmfetch($SESS_DBM, $key)) {
			$expires_at = substr($tmp, 0, strpos($tmp, "|"));
			if ($now > $expires_at) {
				sess_destroy($key);
			}
		}

		$key = dbmnextkey($SESS_DBM, $key);
	}
}

session_set_save_handler(
	"sess_open",
	"sess_close",
	"sess_read",
	"sess_write",
	"sess_destroy",
	"sess_gc");
?>
