<?php

/**
 * the class for xcal config.
 *
 */
class Config {
	 
	public $config;

	/* Constructor :
 	 * get config info from database and initialzise the object
 	 * if the table doesn't exist, show a red message to ask the user to
 	 * set up the plugin in the admin panel
 	 */
	public function __construct($config) {

		$this->config=$config;

		if (!is_array( $config )) 
		{
			throw new Exception ("Config : Array configuration is needed");
		}
	}

	/**
	 * Generic function for getting a config value
	 *
	 * @param string $key config key
	 * @param string $defaultVal default value
	 */
	public function getSection($sectionId)
	{
		if (isset($this->config[$key]))
			return new xcal_Config($this->config[$key]);

		return null;
	}

	/**
	 * Generic function for getting a config value
	 *
	 * @param string $key config key
	 * @param string $defaultVal default value
	 */
	public function getProperty($key,$defaultVal)
	{
		if (isset($this->$key))
			return $this->$key;

		if (isset($this->config[$key]))
			return $this->config[$key];

		return $defaultVal;
	}
	
	function __get($key)
	{
		if (isset($this->config[$key]))
			return $this->$key = $this->config[$key];

		return null;
	}

	function __set($k,$v)
	{
		$this->$k=$this->config[$k]=$v;
	}

}
?>