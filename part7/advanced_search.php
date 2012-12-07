<html>

 <head><title>CS 564 PHP Project Search Result Page</title></head>

 <body>
   <tr>
     <td colspan="2" align="center" valign="top">
      Here are the search results (searched by title):<br>
       <table border="1" width="75%">
        <tr>
         <td align="center" bgcolor="#cccccc"><b>Name</b></td>
         <td align="center" bgcolor="#cccccc"><b>Team</b></td>
         <td align="center" bgcolor="#cccccc"><b>Jersey Number</b></td>
	 <td align="center" bgcolor="#cccccc"><b>Touchdowns</b></td>
         <td align="center" bgcolor="#cccccc"><b>Yards</b></td>
        </tr>	
 <?php
   // First check the itemid to see if it has been set
  $numItems = 0;
  $problems = false;
  $query = "SELECT * FROM fantasy_football.offense";
  if (isset($_POST['namebox']) && $_POST['namebox'] == 'namebox') {
    if (isset($_POST['name']) && $_POST['name'] != ''){
	$numItems++;
	$query .= " WHERE pname = '".$_POST['name'];
    } else {
	$problems = true;
    }
  }
  if (isset($_POST['jerseybox']) && $_POST['jerseybox'] == 'jerseybox') {
    if (isset($_POST['jersey']) && $_POST['jersey'] != ''){
	$numItems++;
        if ($numItems > 1) {
	   $query .= " AND jersey = '".$_POST['jersey'];
        } else {
	   $query .= " WHERE jersey = '".$_POST['jersey'];
	}
    } else {
	$problems = true;
    }
  }
  if (isset($_POST['tdbox']) && $_POST['tdbox'] == 'tdbox') {
    if (isset($_POST['touchdowns']) && $_POST['touchdowns'] != '' && is_numeric($_POST['touchdowns']){
	$numItems++;
        if ($numItems > 1) {
	   switch($_POST['tddrop'])
	    {
	      case "tdlt": $query .= " AND touchdowns < '".$_POST['touchdowns']; break;
	      case "tdeq": $query .= " AND touchdowns = '".$_POST['touchdowns']; break;
	      case "tdgt": $query .= " AND touchdowns > '".$_POST['touchdowns']; break;
	    }
        } else {
	   switch($_POST['tddrop'])
	    {
	      case "tdlt": $query .= " WHERE touchdowns < '".$_POST['touchdowns']; break;
	      case "tdeq": $query .= " WHERE touchdowns = '".$_POST['touchdowns']; break;
	      case "tdgt": $query .= " WHERE touchdowns > '".$_POST['touchdowns']; break;
	    }
	}
    } else {
	$problems = true;
    }
  }
  if (isset($_POST['teambox']) && $_POST['teambox'] == 'teambox') {
    if (isset($_POST['team']) && $_POST['team'] != ''){
	$numItems++;
        if ($numItems > 1) {
	   $query .= " AND nflteam = '".$_POST['team'];
        } else {
	   $query .= " WHERE nflteam = '".$_POST['team'];
	}
    } else {
	$problems = true;
    }
  }
  if (isset($_POST['yardbox']) && $_POST['yardbox'] == 'yardbox') {
    if (isset($_POST['yards']) && $_POST['yards'] != '' && is_numeric($_POST['yards']){
	$numItems++;
        if ($numItems > 1) {
	   switch($_POST['yarddrop'])
	    {
	      case "yardlt": $query .= " AND yards < '".$_POST['yards']; break;
	      case "yardeq": $query .= " AND yards = '".$_POST['yards']; break;
	      case "yardgt": $query .= " AND yards > '".$_POST['yards']; break;
	    }
        } else {
	   switch($_POST['yarddrop'])
	    {
	      case "yardlt": $query .= " WHERE yards < '".$_POST['yards']; break;
	      case "yardeq": $query .= " WHERE yards = '".$_POST['yards']; break;
	      case "yardgt": $query .= " WHERE yards > '".$_POST['yards']; break;
	    }
	}
    } else {
	$problems = true;
    }
  }



/*echo "  <h3><i>Error, title not set to an acceptable value</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/halit/index.html\">Back to main page</a>\n".
	" </body>\n</html>\n";
    exit();*/

  // Connect to the Database
  pg_connect('dbname=cs564_f12 host=postgres.cs.wisc.edu') 
	or die ("Couldn't Connect ".pg_last_error()); 

  // Execute the query and check for errors
  $result = pg_query($query);
  if (!$result) {
    $errormessage = pg_last_error();
    echo "Error with query: " . $errormessage;
    exit();
  }
  
  
  // get each row and print it out  
  while($row = pg_fetch_array($result,NULL,PGSQL_ASSOC))  {
    echo "        <tr>";
    echo "\n         <td align=\"center\">";
    echo "\n          ".$row['pname'];
    echo "\n         </td>";
    echo "\n         <td align=\"center\">";

    echo "\n          ".$row['nflteam'];
    echo "\n         </td>";
    echo "\n         <td align=\"center\">";
    echo "\n          ".$row['jersey'];
    echo "\n         </td>";
    echo "\n         <td align=\"center\">";
    echo "\n          ".$row['touchdowns'];
    echo "\n         </td>";
    echo "\n         <td align=\"center\">";
    echo "\n          ".$row['yards'];
    echo "\n         </td>";
    echo "\n        </tr>";
  }
  pg_close();
?>
 </table>
     </td>
    </tr>
        <?php echo "<a href=\"https://cs564.cs.wisc.edu/rasmusse/index.html\">Back to main page</a>\n"?>
 </body>

</html>
