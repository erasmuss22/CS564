<html>

 <head><title>Update NFL Offensive Player Result Page</title></head>

 <body>      
	
 <?php

   // First check the primary key items were set
  if ( (!isset($_POST['pname']) || $_POST['pname'] == '') ||
       (!isset($_POST['nflteam']) || $_POST['nflteam']) == '' ||
       (!isset($_POST['jersey']) || $_POST['jersey'] == '') ) {
    echo "  <h3><i>Error, you need to specify a player name, team, and jersey number.</i></h3>\n".
      " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
	  " </body>\n</html>\n";
    exit();
  }

  //Make sure two numeric attributes are numeric
  if ( isset($_POST['yards']) && !($_POST['yards'] == '') ) {
    if (!is_numeric($_POST['yards']) ){
      echo "  <h3><i>The yards attribute must be a number.</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
	    " </body>\n</html>\n";
      exit();
    }
  }  

  if ( isset($_POST['touchdowns']) && !($_POST['touchdowns'] == '') ) {
    if (!is_numeric($_POST['touchdowns']) ){
      echo "  <h3><i>The touchdowns attribute must be a number.</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
	    " </body>\n</html>\n";
      exit();
    }
  }
  
  // grab the variables from the fields
  $pname = $_POST['pname'];
  $nflteam = $_POST['nflteam'];
  $jersey = $_POST['jersey'];
  $yards = $_POST['yards'];
  $touchdowns = $_POST['touchdowns'];
  
  // Connect to the Database
  pg_connect('dbname=cs564_f12 host=postgres.cs.wisc.edu') 
	or die ("Couldn't Connect ".pg_last_error()); 
  
  // Set the query to update touchdowns and yards values for a player
  if( strlen($yards) > 0 || strlen($touchdowns) > 0 )	{
	
	$query = "update fantasy_football.offense set";

	if( strlen($yards) )	{
	$query .= " yards='".$yards."'";
	}

	if( strlen($yards) && strlen($touchdowns) )	{
	$query .= ",";
	}

	if( strlen($touchdowns) )	{
	$query .= " touchdowns=".$touchdowns;
	}

	$query .= " where (pname='".$pname."')";
	$query .= " and (nflteam='".$nflteam."')";
	$query .= " and (jersey='".$jersey."')";
	
  }else	{
    echo "  <h3><i>You didn't enter anything to be updated</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
	" </body>\n</html>\n";
    exit();
  }
  
  // Execute the query and check for errors
  $result = pg_query($query);
  if (!$result) {
    $errormessage = pg_last_error();
    echo "Error with query: " . $errormessage;
    exit();
  }
  
  //If there is an affected row, then we found the player to update
  if (pg_affected_rows($result) > 0) {
    echo "  <h3>Update Successful</h3>";
    echo "  <p> ".$pname."'s stats were successfully updated. </p>";
  }
  else {
    echo "  <h3><i>No players matched the entered player name, team, and jersey number.</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
	" </body>\n</html>\n";
    exit();
  }
  
  pg_close();
?>
        <?php echo "<a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to update page</a><br>"?>
        <?php echo "<a href=\"https://cs564.cs.wisc.edu/rasmusse/index.html\">Back to main page</a>\n"?>
 </body>

</html>
