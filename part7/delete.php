<html>

 <head><title>Delete NFL Offensive Player Result Page</title></head>

 <body>      
	
 <?php
   // First check the primary key items were set
  if ( (!isset($_POST['pname']) || $_POST['pname'] == '') ||
       (!isset($_POST['nflteam']) || $_POST['nflteam'] == '') ||
       (!isset($_POST['jersey']) || $_POST['jersey'] == '') ) {
    echo "  <h3><i>Error, you need to specify a value for the player name, team, and jersey number.</i></h3>\n".
      " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
	  " </body>\n</html>\n";
    exit();
  }
  
  // grab those variables
  $pname = $_POST['pname'];
  $nflteam = $_POST['nflteam'];
  $jersey = $_POST['jersey'];
  
  // Connect to the Database
  pg_connect('dbname=cs564_f12 host=postgres.cs.wisc.edu') 
	or die ("Couldn't Connect ".pg_last_error()); 
  

  // first do a query to delete from the offense table
  $query = "delete from fantasy_football.offense ";
  $query .= " where (pname='".$pname."')";
  $query .= " and (nflteam='".$nflteam."')";
  $query .= " and (jersey='".$jersey."')";
  
  // Execute the query and check for errors
  $result = pg_query($query);
  if (!$result) {
    $errormessage = pg_last_error();
    echo "Error with query: " . $errormessage;
    exit();
  }
  

  // next execute a query to delete from players since
  // this is an ISA relationship
  $query = "delete from fantasy_football.players ";
  $query .= " where (pname='".$pname."')";
  $query .= " and (nflteam='".$nflteam."')";
  $query .= " and (jersey='".$jersey."')";
  
  // Execute the query and check for errors
  $result = pg_query($query);
  if (!$result) {
    $errormessage = pg_last_error();
    echo "Error with query: " . $errormessage;
    exit();
  } 

  // See if there was a change in the DB and output the
  // result
  if (pg_affected_rows($result) > 0) {  
     echo "<h3>Succesfully deleted ".$pname."</h3><br>";
  } else {
     echo "<h3>Did not find ".$pname.". No player deleted</h3><br>";
  }

  pg_close();
  ?>
        <?php echo "<a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to update page</a><br>"?>
        <?php echo "<a href=\"https://cs564.cs.wisc.edu/rasmusse/index.html\">Back to main page</a>\n"?>
 </body>

</html>
