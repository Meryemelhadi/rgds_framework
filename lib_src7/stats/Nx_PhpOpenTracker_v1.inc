<?php
//
// phpOpenTracker - The Website Traffic and Visitor Analysis Solution
//
// Copyright 2000 - 2005 Sebastian Bergmann. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// Load phpOpenTracker
#require_once('../../../nx/NxSite.inc');
#include NX_EXT_LIB.'phpOpenTracker.php';
include 'phpOpenTracker.php';

// Load template library
require_once NX_LIB.'stats/MyTemplate.php';

class Nx_PhpOpenTracker 
{
	// cosntructor
	function Nx_PhpOpenTracker(&$app,$monthNames=null)
	{
		// set app for access to properties and templates.
		$this->app=&$app;
		
		// set months
		if ($monthNames==null)
			$monthNames=  array(
				  'January',
				  'February',
				  'March',
				  'April',
				  'May',
				  'June',
				  'July',
				  'August',
				  'September',
				  'October',
				  'November',
				  'December'
				);
		$this->monthNames=$monthNames;
		$this->tplDir=NX_LIB."stats/";
	}

	/* display paths graph */
	function clickPaths($clients) 
	{
		// if no client list provided, use the one in POT config
		if ($clients==null)
		{
			$clients=$config['clients'];
		}
		
		//  get default client (for all pages except access)
		reset($clients);
		$clientLabel=current($clients);
		$client_id=key($clients);
		if (sizeof($clients==1))
		{
			$clientID = $client_id;
		}
		else
			$clientID = isset($_GET['client_id']) ? $_GET['client_id'] : $client_id;
		
		$graph = phpOpenTracker::get(
		array(
		'client_id' => $clientID,
		'api_call' => 'all_paths',
		'result_format' => 'graphviz_object'
		)
		);
		$graph->image('png');
	}

	/* display traffic report */
	function report($clients,$limit=10) 
	{
set_time_limit(0);

// Get references to phpOpenTracker's configuration
// and database objects
$config = &phpOpenTracker_Config::getConfig();
$db     = &phpOpenTracker_DB::getInstance();

// set debug mode
if (isset($_REQUEST['debug'])) {
  phpOpenTracker_Config::set('debug_level', 2);
}

// if no client list provided, use the one in POT config
if ($clients==null)
{
	$clients=$config['clients'];
}

//  get default client (for all pages except access)
reset($clients);
$clientLabel=current($clients);
$client_id=key($clients);
if (sizeof($clients==1))
{
	$clientID = $client_id;
}
else
	$clientID = isset($_GET['client_id']) ? $_GET['client_id'] : $client_id;

// get page parameters
$page     = isset($_GET['nxa'])      ? $_GET['nxa']      : 'access_statistics';
$day      = isset($_GET['day'])       ? $_GET['day']       : date('j');
$month    = isset($_GET['month'])     ? $_GET['month']     : date('n');
$year     = isset($_GET['year'])      ? $_GET['year']      : date('Y');

$buffer = '';
$time   = time();

// set time constraints
switch ($page) {
  case 'total': {
    $start = mktime(0,   0,  0, $month, 1, $year-1);
    $end   = mktime(23, 59, 59, $month, date('t', $start), $year);
    $start = false;
    $end   = false;
  }
  break;

  case 'month': {
    $start = mktime(0,   0,  0, $month, 1, $year);
    $end   = mktime(23, 59, 59, $month, date('t', $start), $year);
  }
  break;

  case 'day': {
    $start = mktime(0,   0,  0, $month, $day, $year);
    $end   = mktime(23, 59, 59, $month, $day, $year);
  }
  break;
}

/* get page hits */
if (isset($start)) {
  $totalPageImpressions = phpOpenTracker::get(
    array(
      'client_id' => $clientID,
      'api_call'  => 'page_impressions',
      'start'     => $start,
      'end'       => $end
    )
  );

  $totalVisits = phpOpenTracker::get(
    array(
      'client_id' => $clientID,
      'api_call'  => 'visits',
      'start'     => $start,
      'end'       => $end
    )
  );

  /* get most visited items */
  $top = $this->top($clientID, $limit, $start, $end);

  list(
    $top['search_engines']['top_items'],
    $top['search_keywords']['top_items'],
    $top['search_engines']['unique_items'],
    $top['search_keywords']['unique_items'],
  ) =

  $this->searchEngines(
    $clientID,
    $start,
    $end,
    $limit
  );
}

switch ($page) {
  case 'access_statistics': {
    $clientIDs = array();

    // Get 'client_list' template
    $content = new MyTemplate($this->tplDir.'templates/client_list.htm');

    if (sizeof($clients)==1)
    {
    	reset($clients);
    	$label=current($clients);
    	$cid=key($clients);
    	   		
    	// display client
		$buffer .= $this->dispClient($cid,$label);   	
    }
    else
    {
    	// all clients
    	
	    // Query database for visited clients
	    $db->query(
	      sprintf(
	        'SELECT DISTINCT(client_id) AS client_id
	           FROM %s
	          ORDER BY client_id',
	
	        $config['visitors_table']
	      )
	    );
	
	    // display only selected clients (in parameters or in config)
	    while ($row = $db->fetchRow()) 
	    {
	      $cid = $row['client_id'];
	      
	      if (isset($clients[$cid]))
	      {
		    	// get client label
	    		$label=$clients[$cid];
		    		
		    	// display client
				$buffer .= $this->dispClient($cid,$label);   	
	      }	            	
	    }
    }
    
    // Merge buffer into content template
    $content->setVar('clients', $buffer);
  }
  break;

  /* ============= CURRENT ACTIVITY =============== */

  case 'current_activity': {
  	
  	
    // Get 'current_activity' template
    $content = new MyTemplate($this->tplDir.'templates/current_activity.htm');

    $visitors = array();

    foreach ($clients as $clientID => $client) {
      $resultSet = phpOpenTracker::get(
        array(
          'client_id' => $clientID,
          'api_call'  => 'visitors_online'
        )
      );

      if ($resultSet) {
        foreach ($resultSet as $resultItem) {
          $index = sizeof($visitors);

          $visitors[$index]['last_access']  = $resultItem['last_access'];
          $visitors[$index]['site']         = $client;
          $visitors[$index]['document']     = $resultItem['clickpath']->documents[sizeof($resultItem['clickpath']->documents)-1];
          $visitors[$index]['document_url'] = $resultItem['clickpath']->document_urls[sizeof($resultItem['clickpath']->document_urls)-1];
          $visitors[$index]['host']         = $resultItem['host'];
          $visitors[$index]['referer']      = $resultItem['referer'];
        }
      }
    }

    if (!empty($visitors)) {
      foreach($visitors as $visitor) $tmp[] = $visitor['last_access'];
      array_multisort($tmp, SORT_DESC, $visitors);

      foreach ($visitors as $visitor) {
        $item = new MyTemplate($this->tplDir.'templates/visitor.htm');

        $item->setVar(
          array(
            'last_access',
            'site',
            'document',
            'document_url',
            'host',
            'referer'
          ),

          array(
            date('d-m-Y H:i:s', $visitor['last_access']),
            $visitor['site'],
            $visitor['document'] != $visitor['document_url'] ? $visitor['document'] : 'not available',
            $visitor['document_url'],
            $visitor['host'],
            $visitor['referer'] != '' ? $visitor['referer'] : 'not available'
          )
        );

        $buffer .= $item->parse();
      }
    }

    $content->setVar('visitors', $buffer);
  }
  break;

  /* ============= TOTAL =============== */
  case 'total': {
    // Get 'total' template
    $content = new MyTemplate($this->tplDir.'templates/total.htm');

    // Query first and last date for this client
    $db->query(
      sprintf(
        "SELECT MIN(timestamp) AS first_access
           FROM %s
          WHERE client_id = '%d'",

        $config['visitors_table'],
        $clientID
      )
    );

    $row = $db->fetchRow();

    $month = $firstMonth = date('n', $row['first_access']);
    $year  = $firstYear  = date('Y', $row['first_access']);
    $lastMonth           = date('n', time());
    $lastYear            = date('Y', time());

    // Loop through months from first to last access
    while ($year <= $lastYear) {
      // Get monthly_statistics template
      $itemMonthlyStatistics = new MyTemplate($this->tplDir.'templates/monthly_statistics.htm');

      // Get start and end timestamp for this month
      $start = mktime( 0,  0,  0, $month, 1, $year);
      $end   = mktime(23, 59, 59, $month, date('t', $start), $year);

      // Query Page Impressions for this client and month
      $pi = phpOpenTracker::get(
        array(
          'client_id' => $clientID,
          'api_call'  => 'page_impressions',
          'start'     => $start,
          'end'       => $end
        )
      );

      // Query Visits for this client and month
      $visits = phpOpenTracker::get(
        array(
          'client_id' => $clientID,
          'api_call'  => 'visits',
          'start'     => $start,
          'end'       => $end
        )
      );

      // Generate link to month statistics page
      $_month = sprintf(
        '<a href="?page=month&amp;client_id=%s&amp;month=%s&amp;year=%s">%s</a>',

        $clientID,
        $month,
        $year,
        $this->monthNames[($month-1)] . ' ' . $year
      );

      // Fill in monthly_statistics template variables
      $itemMonthlyStatistics->setVar(
        array(
          'client',
          'pi_number',
          'pi_percent',

          'visits_number',
          'visits_percent',

          'month'
        ),

        array(
          $clientLabel,
          $pi,
          $totalPageImpressions ? number_format(((100 * $pi) / $totalPageImpressions), 2) : 0,

          $visits,
          $totalVisits ? number_format(((100 * $visits) / $totalVisits), 2) : 0,

          $_month
        )
      );

      // Parse template to buffer
      $buffer .= $itemMonthlyStatistics->parse();

      if ($month == $lastMonth && $year == $lastYear) {
        break;
      }

      if ($month < 12) {
        $month++;
      } else {
        $month = 1;
        $year++;
      }
    }

    // Fill in content template variables
    $content->setVar(
      array(
        'client',
        'first',
        'last',
        'monthly_statistics'
      ),

      array(
      	$clientLabel,
        date('d-M-Y', isset($firstAccess) ? $firstAccess : $time),
        date('d-M-Y', isset($lastAccess)  ? $lastAccess  : $time),
        $buffer
      )
    );
  }
  break;

  case 'month': {
    // Get month template
    $content = new MyTemplate($this->tplDir.'templates/month.htm');

    // Query Page Impressions for this client and each day of this month
    $pi = phpOpenTracker::get(
      array(
        'client_id' => $clientID,
        'api_call'  => 'page_impressions',
        'start'     => $start,
        'end'       => $end,
        'interval'  => 86400
      )
    );

    // Query visits for this client and each day of this month
    $visits = phpOpenTracker::get(
      array(
        'client_id' => $clientID,
        'api_call'  => 'visits',
        'start'     => $start,
        'end'       => $end,
        'interval'  => 86400
      )
    );

    // Loop through days
    for ($i = 0; $i < sizeof($pi); $i++) 
    {
    	if ($pi[$i]['value']==0)
    		continue;
    		
      // Get daily_statistics template
      $itemDailyStatistics = new MyTemplate($this->tplDir.'templates/daily_statistics.htm');

      // Generate link to day statistics page
      $_day = sprintf(
        '<a href="?page=day&amp;client_id=%s&amp;day=%s&amp;month=%s&amp;year=%s">%s</a>',

        $clientID,
        $i + 1,
        $month,
        $year,
        $i + 1
      );

      // Fill in template variables
      $itemDailyStatistics->setVar(
        array(
          'day',
          'pi_number',
          'pi_percent',
          'visits_number',
          'visits_percent'
        ),

        array(
          $_day,
          $pi[$i]['value'],
          $totalPageImpressions ? number_format(((100 * $pi[$i]['value']) / $totalPageImpressions), 2) : '0',
          $visits[$i]['value'],
          $totalVisits ? number_format(((100 * $visits[$i]['value']) / $totalVisits), 2) : '0'
        )
      );

      // Parse template to buffer
      $buffer .= $itemDailyStatistics->parse();
    }

    // Fill in content template variables
    $content->setVar(
      array(
        'client',
        'daily_statistics',
        'month',
        'start',
        'end',
        'year'
      ),

      array(
      	$clientLabel,
        $buffer,
        $this->monthNames[$month - 1],
        $start,
        $end,
        $year
      )
    );
  }
  break;

  case 'day': {
    // Get day template
    $content = new MyTemplate($this->tplDir.'templates/day.htm');

    // Query Page Impressions for this client and each day of this day
    $pi = phpOpenTracker::get(
      array(
        'client_id' => $clientID,
        'api_call'  => 'page_impressions',
        'start'     => $start,
        'end'       => $end,
        'interval'  => 3600
      )
    );

    // Query Visits for this client and each day of this day
    $visits = phpOpenTracker::get(
      array(
        'client_id' => $clientID,
        'api_call'  => 'visits',
        'start'     => $start,
        'end'       => $end,
        'interval'  => 3600
      )
    );

    // Loop through hours
    for ($i = 0; $i < sizeof($pi); $i++) {
		if ($pi[$i]['value']==0)
			continue;
    		
      $hour = sprintf(
        '%02d:00 - %02d:00',

        $i,
        (($i + 1) < 24) ? ($i + 1) : 0
      );

      // Get hourly_statistics template
      $itemDailyStatistics = new MyTemplate($this->tplDir.'templates/hourly_statistics.htm');

      // Fill in template variables
      $itemDailyStatistics->setVar(
        array(
          'hour',
          'pi_number',
          'pi_percent',
          'visits_number',
          'visits_percent'
        ),

        array(
          $hour,
          $pi[$i]['value'],
          $totalPageImpressions ? number_format(((100 * $pi[$i]['value']) / $totalPageImpressions), 2) : '0',
          $visits[$i]['value'],
          $totalVisits ? number_format(((100 * $visits[$i]['value']) / $totalVisits), 2) : '0'
        )
      );

      // Parse template to buffer
      $buffer .= $itemDailyStatistics->parse();
    }

    // Fill in content template variables
    $content->setVar(
      array(
        'daily_statistics',
        'month',
        'day',
        'start',
        'end',
        'year'
      ),

      array(
        $buffer,
        $this->monthNames[$month - 1],
        $day,
        $start,
        $end,
        $year
      )
    );
  }
  break;

  default: {
    header('Location: index.php');
    exit;
  }
}

// Fill in content template variables
$content->setVar(
  array(
    'client_id',

    'entry_pages',
    'exit_pages',
    'exit_targets',
    'hosts',
    'pages',
    'referers',
    'operating_systems',
    'user_agents',
    'search_engines',
    'search_keywords',

    'total_entry_pages',
    'total_exit_pages',
    'total_exit_targets',
    'total_hosts',
    'total_pages',
    'total_referers',
    'total_operating_systems',
    'total_user_agents',
    'total_search_engines',
    'total_search_keywords',

    'total_pi',
    'total_visits',

    'limit'
  ),

  array(
    $clientID,

    isset($top['entry_pages']['top_items'])           ? $top['entry_pages']['top_items']          : '',
    isset($top['exit_pages']['top_items'])            ? $top['exit_pages']['top_items']           : '',
    isset($top['exit_targets']['top_items'])          ? $top['exit_targets']['top_items']         : '',
    isset($top['hosts']['top_items'])                 ? $top['hosts']['top_items']                : '',
    isset($top['pages']['top_items'])                 ? $top['pages']['top_items']                : '',
    isset($top['referers']['top_items'])              ? $top['referers']['top_items']             : '',
    isset($top['operating_systems']['top_items'])     ? $top['operating_systems']['top_items']    : '',
    isset($top['user_agents']['top_items'])           ? $top['user_agents']['top_items']          : '',
    isset($top['search_engines']['top_items'])        ? $top['search_engines']['top_items']       : '',
    isset($top['search_keywords']['top_items'])       ? $top['search_keywords']['top_items']      : '',

    isset($top['entry_pages']['unique_items'])        ? $top['entry_pages']['unique_items']       : 0,
    isset($top['exit_pages']['unique_items'])         ? $top['exit_pages']['unique_items']        : 0,
    isset($top['exit_targets']['unique_items'])       ? $top['exit_targets']['unique_items']      : 0,
    isset($top['hosts']['unique_items'])              ? $top['hosts']['unique_items']             : 0,
    isset($top['pages']['unique_items'])              ? $top['pages']['unique_items']             : 0,
    isset($top['referers']['unique_items'])           ? $top['referers']['unique_items']          : 0,
    isset($top['operating_systems']['unique_items'])  ? $top['operating_systems']['unique_items'] : 0,
    isset($top['user_agents']['unique_items'])        ? $top['user_agents']['unique_items']       : 0,
    isset($top['search_engines']['unique_items'])     ? $top['search_engines']['unique_items']    : 0,
    isset($top['search_keywords']['unique_items'])    ? $top['search_keywords']['unique_items']   : 0,

    isset($totalPageImpressions)                      ? $totalPageImpressions                     : 0,
    isset($totalVisits)                               ? $totalVisits                              : 0,

    $limit
  )
);


	// Merge content template into layout template
	$this->app->addProperties(
		array (
			'page.content' => $content->parse(),
			'client' => $clientLabel,
			'pot_version' => PHPOPENTRACKER_VERSION
		)
	);
}

function searchEngines($clientID, $start, $end, $limit) 
{
  $searchEngines       = '';
  $searchKeywords      = '';
  $totalSearchEngines  = 0;
  $totalSearchKeywords = 0;

  if (phpOpenTracker_API::pluginLoaded('search_engines')) {
    $result = phpOpenTracker::get(
      array(
          'client_id' => $clientID,
          'api_call'  => 'search_engines',
          'what'      => 'top_search_engines',
          'start'     => $start,
          'end'       => $end,
          'limit'     => $limit
      )
    );

    $totalSearchEngines = $result['unique_items'];

    for ($i = 0; $i < sizeof($result['top_items']); $i++) {
      // Get item template
      $item = new MyTemplate($this->tplDir.'templates/item.htm');

      // Fill in item template variables
      $item->setVar(
        array(
          'rank',
          'count',
          'percent',
          'string'
        ),

        array(
          $i + 1,
          $result['top_items'][$i]['count'],
          $result['top_items'][$i]['percent'],
          $result['top_items'][$i]['string']
        )
      );

      $searchEngines .= $item->parse();
    }

    $result = phpOpenTracker::get(
      array(
          'client_id' => $clientID,
          'api_call'  => 'search_engines',
          'what'      => 'top_search_keywords',
          'start'     => $start,
          'end'       => $end,
          'limit'     => $limit
      )
    );

    $totalSearchKeywords = $result['unique_items'];

    for ($i = 0; $i < sizeof($result['top_items']); $i++) {
      // Get item template
      $item = new MyTemplate($this->tplDir.'templates/item.htm');

      // Fill in item template variables
      $item->setVar(
        array(
          'rank',
          'count',
          'percent',
          'string',
          'url'
        ),

        array(
          $i + 1,
          $result['top_items'][$i]['count'],
          $result['top_items'][$i]['percent'],
          $result['top_items'][$i]['string'],
          $result['top_items'][$i]['url']
        )
      );

      $searchKeywords .= $item->parse();
    }
  }

  return array(
    $searchEngines,
    $searchKeywords,
    $totalSearchEngines,
    $totalSearchKeywords
  );
}

function top($clientID, $limit, $start = false, $end = false) 
{
  $batchKeys = array(
    'pages',
    'entry_pages',
    'exit_pages',
    'exit_targets',
    'hosts',
    'referers',
    'operating_systems',
    'user_agents'
  );

  $batchWhat = array(
    'document',
    'entry_document',
    'exit_document',
    'exit_target',
    'host',
    'referer',
    'operating_system',
    'user_agent'
  );
  
  $batchTPL = array(
    'item_url',
    'item_url',
    'item_url',
    'item_url',
    'item',
    'item',
    'item',
    'item',
  );
  $batchResult = array();

  // Loop through $batchKeys / $batchWhat
  for ($i = 0; $i < sizeof($batchKeys); $i++) {
    // Query Top <$limit> items of category <$batchWhat[$i]>
    $result = phpOpenTracker::get(
      array(
        'client_id' => $clientID,
        'api_call'  => 'top',
        'what'      => $batchWhat[$i],
        'start'     => $start,
        'end'       => $end,
        'limit'     => $limit
      )
    );

    for ($j = 0; $j < sizeof($result['top_items']); $j++) {
      // Get item template
      $tpl= $batchTPL[$i];
      $item = new MyTemplate($this->tplDir."templates/{$tpl}.htm");

      // Fill in item template variables
      $item->setVar(
        array(
          'rank',
          'count',
          'percent',
          'string',
          'url'
        ),

        array(
          $j + 1,
          $result['top_items'][$j]['count'],
          $result['top_items'][$j]['percent'],
          $result['top_items'][$j]['string'],
          $result['top_items'][$j]['url']
        )
      );

      if (!isset($batchResult[$batchKeys[$i]]['top_items'])) {
        $batchResult[$batchKeys[$i]]['top_items'] = '';
      }

      $batchResult[$batchKeys[$i]]['top_items'] .= $item->parse();
    }

    $batchResult[$batchKeys[$i]]['unique_items'] = $result['unique_items'];
  }

  return $batchResult;
}

function dispClient($clientID,$clientLabel) 
{
	       // Get client template
        $client = new MyTemplate($this->tplDir.'templates/client.htm');

        // Fill in client template variables
        $client->setVar(
          array(
            'client',
            'client_id',

            'pi_total',
            'pi_month',
            'visits_total',
            'visits_month'
          ),

          array(
            $clientLabel,
            $clientID,

            // Total Page Impressions for current client
            phpOpenTracker::get(
              array(
                'client_id' => $clientID,
                'api_call'  => 'page_impressions',
                'range'     => 'total'
              )
            ),

            // Page Impressions this month for current client
            phpOpenTracker::get(
              array(
                'client_id' => $clientID,
                'api_call'  => 'page_impressions',
                'range'     => 'current_month'
              )
            ),

            // Total Visits for current client
            phpOpenTracker::get(
              array(
                'client_id' => $clientID,
                'api_call'  => 'visits',
                'range'     => 'total'
              )
            ),

            // Visits this month for current client
            phpOpenTracker::get(
              array(
                'client_id' => $clientID,
                'api_call'  => 'visits',
                'range'     => 'current_month'
              )
            )
          )
        );

        // Parse template to buffer
        return $client->parse();
	}
}
?>
