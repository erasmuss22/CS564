<html>

 <head><title>Insert NFL Offensive Player Result Page</title></head>

 <body>      
	
 <?php
   // First check the primary key items were set
  if ( (!isset($_POST['pname']) || $_POST['pname'] == '') ||
       (!isset($_POST['nflteam']) || $_POST['nflteam'] == '') ||
       (!isset($_POST['jersey']) || $_POST['jersey'] == '') ||
       (!isset($_POST['yards']) || $_POST['yards'] == '') ||
	   (!isset($_POST['touchdowns']) || $_POST['touchdowns'] == '') ) {
    echo "  <h3><i>Error, you need to specify a value for the player name, team".
	  ", jersey number, total yards, and touchdowns</i></h3>\n".
      " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
	  " </body>\n</html>\n";
    exit();
  }
  //Make sure two numeric attributes are numeric
  if (!is_numeric($_POST['yards']) || !is_numeric($_POST['touchdowns']) ){
    echo "  <h3><i>The yards, and touchdowns attribute must be a number.</i></h3>\n".
	  " <a href=\"https://cs564.cs.wisc.edu/rasmusse/update.html\">Back to Update Player page.</a>\n".
      " </body>\n</html>\n";
    exit();
  }
  
  $pname = $_POST['pname'];
  $nflteam = $_POST['nflteam'];
  $jersey = $_POST['jersey'];
  $yards = $_POST['yards'];
  $touchdowns = $_POST['touchdowns'];
  
  // Connect to the Database
  pg_connect('dbname=cs564_f12 host=postgres.cs.wisc.edu') 
	or die ("Couldn't Connect ".pg_last_error()); 
  
  
  $query = "insert into fantasy_football.offense (pname, nflteam, jersy, yards, touchdowns) "
  $query .= "values ('".$pname."','".$nflteam."','".$jersey."',".$yards.",".$touchdowns.")"
  
  // Execute the query and check for errors
  $result = pg_query($query);
  if (!$result) {
    $errormessage = pg_last_error();
    echo "Error with query: " . $errormessage;
    exit();
  }
  
  //Need to check for error inserting?
  
  pg_close();
  ?>

        <?php echo "<a href=\"https://cs564.cs.wisc.edu/rasmusse/index.html\">Back to main page</a>\n"?>
 </body>

</html>