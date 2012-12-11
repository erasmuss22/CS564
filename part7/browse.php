<html>

 <head><title>NFL Offensive Player All Players Page</title></head>

 <body>
   <tr>
     <td colspan="2" align="center" valign="top">
      Here are all offensive players in the NFL:<br>
       <table border="1" width="75%">
        <tr>
         <td align="center" bgcolor="#cccccc"><b>Name</b></td>
         <td align="center" bgcolor="#cccccc"><b>Team</b></td>
         <td align="center" bgcolor="#cccccc"><b>Jersey #</b></td>
	 <td align="center" bgcolor="#cccccc"><b>Touchdowns</b></td>
         <td align="center" bgcolor="#cccccc"><b>Yards</b></td>
        </tr>	
 <?php
 
  // Connect to the Database
  pg_connect('dbname=cs564_f12 host=postgres.cs.wisc.edu') 
	or die ("Couldn't Connect ".pg_last_error()); 

  // Set query to select all from DB
  $query = "SELECT * FROM fantasy_football.offense";

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
