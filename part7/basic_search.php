<html>

 <head><title>NFL Offensive Player Search Result Page</title></head>

 <body>
   <tr>
     <td colspan="2" align="center" valign="top">
      Here are the search results (searched by name):<br>
       <table border="1" width="75%">
        <tr>
         <td align="center" bgcolor="#cccccc"><b>Name</b></td>
         <td align="center" bgcolor="#cccccc"><b>Team</b></td>
         <td align="center" bgcolor="#cccccc"><b>Jersey #</b></td>
	 <td align="center" bgcolor="#cccccc"><b>Touchdowns</b></td>
         <td align="center" bgcolor="#cccccc"><b>Yards</b></td>
        </tr>	
 <?php
   
  // Make sure the user entered a name
  if (! isset($_POST['title'])) {
    echo "  <h3><i>Error, name not set to an acceptable value</i></h3>\n".
        " <a href=\"https://cs564.cs.wisc.edu/rasmusse/index.html\">Back to main page</a>\n".
	" </body>\n</html>\n";
    exit();
  }

  // grab the name and store it in a var
  $title = $_POST['title'];

  // Connect to the Database
  pg_connect('dbname=cs564_f12 host=postgres.cs.wisc.edu') 
	or die ("Couldn't Connect ".pg_last_error()); 

  // Use wild cards to allow for more search flexibility
  $query = "SELECT * FROM fantasy_football.offense where pname LIKE '%".$title."%'";

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
